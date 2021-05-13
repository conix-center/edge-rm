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
app.post('/reduce/:key', function(req, res) {
    if(!req.params.key) return res.status(404).send({success: false, error: "malformed request"});
    let key = req.params.key
    var pathToFile = path.join(__dirname + "/scripts/" + key);
    var stream = req.pipe(fs.createWriteStream(pathToFile))
    stream.on('finish', function() {
        try {
            scripts[key] = require(pathToFile);
            console.log(scripts[key]);
            console.log(scripts[key].reduce);
            res.status(200).send({success: true, key: key});
        } catch(e) {
            res.status(500).send({success: false, error: e});
        } 
    });
    stream.on('error', function(err) {
        res.status(500).send({success: false, error: err});
    });
});

app.post('/reduce/:key/data', jsonParser, function(req, res) {
    console.log(req.body);
    if(!req.params.key) return res.status(404).send({success: false, error: "malformed request"});
    let key = req.params.key
    var scrippie = scripts[key];
    if(!scrippie) return res.status(404).send({success: false, error: "not loaded"});
    if(!scrippie.reduce) return res.status(404).send({success: false, error: "no reduce"});
    scrippie.reduce(req.body);
    res.send("yay!");
});

app.listen(port, () => console.log("Listening on port " + port));

