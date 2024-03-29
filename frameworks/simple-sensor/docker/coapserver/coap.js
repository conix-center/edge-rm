var coap        = require('coap')
  , server      = coap.createServer()

var fs = require('fs')
var path = require('path')
 
var scripts = {}

server.on('request', function(req, res) {
  var pathSplit = req.url.split('/');
  if(pathSplit.length < 4) return res.end("Hello World!");

  var isReduce = pathSplit[1]
  var key  = pathSplit[2];
  var pathType = pathSplit[3];
  // POST /reduce/:key/create or /reduce/:key/data

  if(req.method == "GET") {
    console.log("GET /")
    return res.end("Hello World!")
  }

  var pathToFile = path.join(__dirname + "/scripts/" + key);

  if(req.method == "POST" && pathType == 'create' && key != '') {
    fs.writeFile(pathToFile, req.payload, function(err) {
      if(err) {
        console.log(err)
        return res.end("Failure.")
      }
      try {
          scripts[key] = require(pathToFile);
          console.log(scripts[key]);
          console.log(scripts[key].reduce);
          res.end("Success.")
      } catch(e) {
          res.end("Failure.")
      } 
      res.end("Got it!")
    })
  } else if(req.method == "POST" && pathType == 'data' && key != '') {
    var scrippie = scripts[key];
    if(!scrippie) return res.end("Not loaded.")
    if(!scrippie.reduce) return res.end("No reduce.")
    scrippie.reduce(req.body);
    res.end("OK!");
  } else {
    res.end("Unknown.")
  }
})
 
var port = process.env.SERVER_PORT;
if (!port) {
  // the default CoAP port is 5683
  port = 5683
}

server.listen(port, function() {
  console.log("CoAP server launched on port " + port.toString())
})
