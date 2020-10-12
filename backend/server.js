#!/usr/bin/env node

var express = require('express')
var app = express()
var async = require('async');
var coap        = require('coap');

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

app.post('/startSensor', jsonParser, function(req, res) {

	if(!req.cookies['client']) {
		return res.send("We couldn't find your client id... please go back and reload!");
	}

	log.info("starting tasks")

	var python;
	if(req.body.ffunc) {
		python = spawn('python3', ['../scheduler/sensor-scheduler.py', 
			'--host', '128.97.92.77', 
			'--client', req.cookies['client'], 
			'--sensor', req.body.sensor,
			'--period', req.body.period,
			'--ffunc', req.body.ffunc,
			'--fval', req.body.fval])
	} else {
		python = spawn('python3', ['../scheduler/sensor-scheduler.py', 
			'--host', '128.97.92.77', 
			'--client', req.cookies['client'], 
			'--sensor', req.body.sensor,
			'--period', req.body.period])
	}

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
		res.status(200).send(dataToSend)
	})
})

app.get('/start', jsonParser, function(req, res) {

	if(!req.cookies['client']) {
		return res.send("We couldn't find your client id... please go back and reload!");
	}

	log.info("starting tasks")

	const python = spawn('python3', ['../scheduler/scheduler.py', '--host', '128.97.92.77', '--tasks', 'tasks.json', '--client', req.cookies['client']])
	var dataToSend = '';
	python.stdout.on('data', function(data) {
		//dataToSend += data.toString();
	});
	python.stderr.on('data', function(data) {
		dataToSend += data.toString();
		log.info(dataToSend);
	});
	python.on('close', (code) => {
		log.info(code);
		//res.status(200).send(`<!DOCTYPE html><html><body><p>${dataToSend}</p><form action="/"><input type="submit" value="OK" /></form></body></html>`)
		res.status(200).send(dataToSend)
	})
})

app.get('/stop/all', function(req, res) {
	// log.info(req.body);
	log.info("stopping all tasks")

	const python = spawn('python3', ['../scheduler/kill-task.py', '--host', '128.97.92.77', '--all'])
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
			//add the agent to the list of nodes
			result['nodes'].push({
				id: agents[i].id,
				name: agents[i].name,
				group: group_num
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

	function getCoapData(pathExt, data, callback) {
		var req2 = coap.request({
			host: '128.97.92.77',
			//host: 'localhost',
		  	port: 3002,
		  	pathname: clientID + '-' + pathExt,
		  	//pathname: 'test',
		  	method: "GET"
		});

		console.log("Fetching predictions from: ",clientID+'-'+pathExt);

		req2.on('response', function(res) {
			try {
				jData = JSON.parse(String(res.payload))
			} catch (error) {
				callback(null,data);
				return;
			}

			prefix = ""

			if(pathExt == 't') {
				prefix="Temperature: "
			} else if(pathExt == 'p') {
				prefix="Pressure "
			} else if(pathExt =='h') {
				prefix="Humidity "
			}

			for(var i =0; i < jData.length; i++) {
				jData[i].value = prefix + jData[i].value;
			}

			if(data) {
				data.push(...jData);
				callback(null, data);
			} else {
				callback(null, jData);
			}
		});
		req2.end();
	}

	async.waterfall([
		async.apply(getCoapData,'t',[]),
		async.apply(getCoapData,'p'),
		async.apply(getCoapData,'h')
	], function(error, data){
		if(error) {
			res.status(500).send([]);
		} else {
			//order the data by time
			function custom_sort(a, b) {
			    return Number(a.time) - Number(b.time);
			}
			data.sort(custom_sort);

			//convert it into the final strings
			fStrings = []
			for(var i =0; i < data.length; i++) {
				var newDate = new Date(Number(data[i].time));
				fStrings.push(newDate.toLocaleTimeString()+ ': ' + data[i].value);
			}

			//send it back as an array of strings
			res.status(200).send(JSON.stringify(fStrings));
		}
	})
})

app.get('/predictions', function(req, res) {
	var clientID = req.cookies['client']
	if(!clientID) {
		return res.status(500).send([]);
	}
	log.info("GET /predictions")
	request(`http://128.97.92.77:3003/${clientID}-results.json`).on('error', (e) => {res.status(500).send([])}).pipe(res);
})

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
