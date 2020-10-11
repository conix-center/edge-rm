// const http = require('http');
const fs = require('fs');

const express = require('express');
var app = express();
var path = require('path')

var port = process.env.SERVER_PORT;
if(!port) {
    port = 3000;
}

app.get('/', function(req,res) {
    res.status(200).send({'hello':'world'});
})

app.get('/:filename', function(req, res) {
    var pathToFile = path.join(__dirname + "/files/" + req.params.filename);
    if(fs.existsSync(pathToFile)) {
        return res.status(200).sendFile(pathToFile);
    } else {
        return res.status(404).send('');
    }
})

app.post('/:filename', function(req, res) {
    var pathToFile = path.join(__dirname + "/files/" + req.params.filename);
    var stream = req.pipe(fs.createWriteStream(pathToFile))
    stream.on('finish', function() {
        res.status(200).send("Got it!")
    })
})

app.listen(port, () => console.log("Listening on port " + port));

