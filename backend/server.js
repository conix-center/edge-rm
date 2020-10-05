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

app.get('/agents', function(req, res) {
	log.info("GET /agents")
	request('http://128.97.92.77/').on('error', (e) => {res.status(500).send({})}).pipe(res);
})

app.get('/', function(req,res) {
	// res.status(200).send({'hello':'world'});
	res.status(200).sendFile(path.join(__dirname + '/html/index.html'))
})

// app.listen(3000, () => console.log("Listening on port 3000"))
app.listen(3000, () => log.info("Listening on port 3000"))