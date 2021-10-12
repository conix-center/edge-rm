#!/usr/bin/env node

var express = require('express')
var app = express()
var async = require('async');
var coap        = require('coap');
var fs = require('fs');

var coapTiming = {
  ackTimeout: 1,
  ackRandomFactor: 1.0,
  maxRetransmit: 3,
  maxLatency: 2,
  piggybackReplyMs: 10
};

coap.updateTiming(coapTiming);

var bodyParser = require('body-parser')
var jsonParser = bodyParser.json()

var cookieParser = require('cookie-parser')
app.use(cookieParser())

const {spawn} = require('child_process');
var path = require('path')

// var moment = require('moment-timezone')
const request = require('request');

var winston = require('winston');
const log = winston.createLogger({
	transports: [
		new winston.transports.Console(),
    	new winston.transports.File({ filename: 'output.log' })
	],
	exceptionHandlers: [
		new winston.transports.Console(),
		new winston.transports.File({ filename: 'exceptions.log' })
	]
});

app.post('/startMapReduce', jsonParser, function(req, res) {

	if(!req.cookies['client']) {
		return res.send("We couldn't find your client id... please go back and reload!");
	}

	log.info("starting tasks")

	var mapname = req.cookies['client'] + '_map.c'
	var reducename = req.cookies['client'] + '_reduce.js'

	//Write the map and reduce functions to files
	fs.writeFile(mapname, req.body.map, function(err) {
	  if(err) {
	    res.status(200).send(err);
	    return
	  } else {
	    fs.writeFile(reducename, req.body.reduce, function(err) {
	      if(err) {
		res.status(200).send(err);
		return
	      } else {
		var python;
		console.log(mapname)
		console.log(reducename)
		console.log("Starting with ID", req.cookies['client'])
		python = spawn('python3', ['../map-reduce/map-reduce.py',
			'--host', '128.97.92.77',
			'--sensor', req.body.sensor,
			'--period', req.body.period,
			'--map', mapname,
			'--reduce', reducename,
			'--id', req.cookies['client'],
			'--reissue-tasks']);

		var dataToSend = '';
		python.stdout.on('data', function(data) {
			log.info(data.toString());
		});
		python.stderr.on('data', function(data) {
			dataToSend += data.toString();
			log.info(dataToSend);
		});
		python.on('close', (code) => {
			log.info(code);
			res.status(200).send(dataToSend);
		});
	      }
	    });
	  }
	});
})

app.get('/stop/all', function(req, res) {
	// log.info(req.body);
	log.info("stopping all tasks")

	const python = spawn('python3', ['../../edgerm/tools/kill-task.py', '--host', '128.97.92.77', '--all'])
	var dataToSend = '';
	python.stdout.on('data', function(data) {
		dataToSend += data.toString();
		log.info(dataToSend);
	});
	python.stderr.on('data', function(data) {
		dataToSend += data.toString();
		log.info(dataToSend);
	});
	python.on('close', (code) => {
		log.info(code);
		res.status(200).send(`<!DOCTYPE html><html><body><p>${dataToSend}</p><form action="/"><input type="submit" value="OK" /></form></body></html>`)
	})
})

app.get('/tasks', function(req, res) {
	log.info("GET /tasks")
	request('http://128.97.92.77/tasks').on('error', (e) => {res.status(500).send({})}).pipe(res);
	// res.status(200).sendFile(path.join(__dirname + '/tasks.json'));
})

app.get('/framework', function(req, res) {
	log.info("GET /framework")
	res.status(200).sendFile(path.join(__dirname + '/tasks.json'));
})

app.get('/network.json', function(req, res) {
	request('http://128.97.92.77/', function(error, response, body) {
		var result = {'nodes':[],'links':[]}
		if (error) {
			log.info(error);
			return res.send(result);
		}
		var agents = JSON.parse(body);
		//for every agent
		for (var i = 0; i < agents.length; i++) {

			if ((new Date()).getTime() - agents[i].lastPing > 300000) {
				//device has been offline for 5 minutes. Ignore
				continue
			}

			var group_num = (agents[i].name == "jet" || agents[i].name == "raspberrypi") ? 1 : 5

			//add links to every other agent
			for (var j = 0; j < result['nodes'].length; j++) {
				//only if it's in the same group, or if it's connecting to jet
				if(group_num == result['nodes'][j].group || agents[i].name == "jet" || result['nodes'][j].name == "jet") {
					result['links'].push({
						'source':result['nodes'][j].id,
						'target':agents[i].id,
						'value':1
					})
				}
			}

			var resources = ""
			for (var j = 0; agents[i].resources && j < agents[i].resources.length; j++) {
				var resource = agents[i].resources[j]
				if (resource.scalar) {
					resources += "\nResource: " + resource.name + " = " + resource.scalar.value 
				} else if (resource.type == "DEVICE") {
					resources += "\nDevice: " + resource.name 
				}
			}

			//add the agent to the list of nodes
			result['nodes'].push({
				id: agents[i].id,
				name: agents[i].name,
				group: group_num,
				resources: resources
			})
		}
		//add the master, framework, and client
		var master = {
			id: 'Master',
			name: "Master",
			group:2
		}
		var framework = {
			id: 'CameraFramework',
			name: 'Camera Framework',
			group:3
		}
		var framework2 = {
			id: 'SensorFramework',
			name: 'Sensor Framework',
			group:3
		}
		var client = {
			id: 'Me',
			name: 'You are here',
			group:4
		}
		//connect all agents to the master
		for(var i = 0; i < result['nodes'].length; i++) {
			result['links'].push({
				'source':result['nodes'][i].id,
				'target':master.id,
				'value':10
			})
		}
		result['nodes'].push(master)
		result['nodes'].push(framework)
		result['nodes'].push(framework2)
		result['nodes'].push(client)
		//connect framework to master
		result['links'].push({
			source:framework.id,
			target:master.id,
			value:3
		})
		result['links'].push({
			source:framework2.id,
			target:master.id,
			value:3
		})
		result['links'].push({
			source:client.id,
			target:framework.id,
			value:4
		})
		result['links'].push({
			source:client.id,
			target:framework2.id,
			value:4
		})
		res.send(result);
	});
});

app.get('/agents', function(req, res) {
	log.info("GET /agents")
	request('http://128.97.92.77/').on('error', (e) => {res.status(500).send({})}).pipe(res);
})

app.get('/sensorPredictions', function(req, res) {
	var clientID = req.cookies['client']
	if(!clientID) {
		return res.status(500).send([]);
	}
	log.info("GET /sensorPredictions")

	pathn = '/reduce/' + clientID + '/data';
	console.log(pathn)
	var req2 = coap.request({
		host: '128.97.92.77',
	  	port: 3004,
		pathname: pathn,
	  	method: "GET"
	});

	console.log("Fetching predictions from: ",clientID);

	req2.on('response', function(inres) {
	  //console.log("Got COAP response");
	  //console.log(inres.payload.length);
	  //console.log(String(inres.payload));

	  if(inres.payload.length > 0) {
	    console.log("Coap response:", String(inres.payload))
	    var response = JSON.parse(String(inres.payload));
	    return res.status(200).send(JSON.stringify(response));
	  } else {
	    return res.status(200).send([]);
	  }
	});

	req2.on('error', function(err) {
	  console.log("Error fetching data");
	})

	req2.end()
})

/*app.get('/predictions', function(req, res) {
	var clientID = req.cookies['client']
	if(!clientID) {
		return res.status(500).send([]);
	}
	log.info("GET /predictions")
	request(`http://128.97.92.77:3003/${clientID}-results.json`).on('error', (e) => {res.status(500).send([])}).pipe(res);
})*/

app.get('/clientID', function(req, res) {
	if(req.cookies['client']) {
		res.send(req.cookies['client'])
	} else {
		var cValue = Math.floor(Math.random() * 10000000000)
		res.cookie('client', cValue)
		res.send(cValue)
	}
});

app.get('/', function(req,res) {

	if(!req.cookies['client']) {
		res.cookie('client', Math.floor(Math.random() * 10000000000))
	}

	// res.status(200).send({'hello':'world'});
	res.status(200).sendFile(path.join(__dirname + '/html/index.html'))
})

// app.listen(3000, () => console.log("Listening on port 3000"))
app.listen(3000, () => log.info("Listening on port 3000"))
