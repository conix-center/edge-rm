var express = require('express')
var app = express()

var bodyParser = require('body-parser')
var jsonParser = bodyParser.json()

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

app.get('/start', jsonParser, function(req, res) {
	log.info("starting tasks")

	const python = spawn('python3', ['../scheduler/scheduler.py', '--host', '128.97.92.77', '--tasks', 'tasks.json'])
	var dataToSend = '';
	python.stdout.on('data', function(data) {
		dataToSend += data.toString();
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

app.get('/stop', jsonParser, function(req, res) {
	// log.info(req.body);
	log.info("stopping tasks")

	const python = spawn('python3', ['../scheduler/scheduler_kill.py', '--host', '128.97.92.77', '--tasks', 'tasks.json'])
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
			//add links to every other agent
			for (var j = 0; j < result['nodes'].length; j++) {
				result['links'].push({
					'source':result['nodes'][j].id,
					'target':agents[i].id,
					'value':1
				})
			}
			//add the agent to the list of nodes
			result['nodes'].push({
				id: agents[i].id,
				name: agents[i].name,
				group: 1
			})
		}
		//add the master, framework, and client
		var master = {
			id: 'Master',
			name: "Master",
			group:2
		}
		var framework = {
			id: 'Framework',
			name: 'Framework (this)',
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
			source:client.id,
			target:framework.id,
			value:4
		})
		res.send(result);
	});
});

app.get('/agents', function(req, res) {
	log.info("GET /agents")
	request('http://128.97.92.77/').on('error', (e) => {res.status(500).send({})}).pipe(res);
})

app.get('/predictions', function(req, res) {
	log.info("GET /agents")
	request('http://128.97.92.77:3003/results.json').on('error', (e) => {res.status(500).send([])}).pipe(res);
})

app.get('/', function(req,res) {
	// res.status(200).send({'hello':'world'});
	res.status(200).sendFile(path.join(__dirname + '/html/index.html'))
})

// app.listen(3000, () => console.log("Listening on port 3000"))
app.listen(3000, () => log.info("Listening on port 3000"))
