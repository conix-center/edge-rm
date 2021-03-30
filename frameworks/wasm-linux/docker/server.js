// const http = require('http');
const fs = require('fs');
const path = require('path');

const express = require('express');
const bodyParser = require('body-parser');

var rawParser = bodyParser.raw({type: 'application/octet-stream', limit : '12mb'});

var app = express();

var port = process.env.SERVER_PORT;
if(!port) {
    port = 3002;
}

var task = {};

app.get('/', function(req,res) {
    res.status(200).send({'hello':'world'});
})

app.get('/task', function(req, res) {
	return res.send(task);
})

app.post('/task', rawParser, function(req, res) {
	var filepath = path.join(__dirname, 'wasm', 'task.wasm')
	fs.writeFileSync(filepath, req.body);
	res.sendFile(filepath);
});

app.post('/task/start', function(req, res) {
	
});

app.get('/profile', function(req, res) {
    if (req.query.id) {
        profiles[req.query.id] = req.query;
    } else if (req.query.agent) {
        profiles[req.query.agent] = req.query;
    }
    res.status(200).send("Got it!")
})

app.get('/profile/all', function(req, res) {
    res.status(200).send(profiles)
})

app.listen(port, () => console.log("Listening on port " + port));

