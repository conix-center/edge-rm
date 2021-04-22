// const http = require('http');
const fs = require('fs');
const express = require('express');
const bodyParser = require('body-parser');
var jsonParser = bodyParser.json()
var app = express();
var path = require('path')

var port = process.env.SERVER_PORT;
if(!port) {
    port = 3000;
}

app.get('/', function(req,res) {
    res.status(200).send({'hello':'world'});
})


var scripts = {}
app.post('/reduce', function(req, res) {
    if(!req.query.key) return res.status(404).send({success: false, error: "malformed request"});
    var pathToFile = path.join(__dirname + "/scripts/" + req.query.key);
    var stream = req.pipe(fs.createWriteStream(pathToFile))
    stream.on('finish', function() {
        try {
            scripts[req.query.key] = require(pathToFile);
            console.log(scripts[req.query.key]);
            console.log(scripts[req.query.key].reduce);
            res.status(200).send({success: true, key: req.query.key});
        } catch(e) {
            res.status(500).send({success: false, error: e});
        } 
    });
    stream.on('error', function(err) {
        res.status(500).send({success: false, error: err});
    });
});

app.post('/reduce/data', jsonParser, function(req, res) {
    console.log(req.body);
    if(!req.query.key) return res.status(404).send({success: false, error: "malformed request"});
    console.log(scripts)
    var scrippie = scripts[req.query.key];
    console.log(scrippie)
    if(!scrippie) return res.status(404).send({success: false, error: "not loaded"});
    if(!scrippie.reduce) return res.status(404).send({success: false, error: "no reduce"});
    scrippie.reduce(req.body);
    res.send("yay!");
});

app.listen(port, () => console.log("Listening on port " + port));

