// const http = require('http');
const fs = require('fs');

const express = require('express');
var app = express();

var port = process.env.SERVER_PORT;
if(!port) {
    port = 3001;
}

var profiles = {};

app.get('/', function(req,res) {
    res.status(200).send({'hello':'world'});
})

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

