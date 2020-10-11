var coap        = require('coap')
  , server      = coap.createServer()

var fs = require('fs')
var path = require('path')

var latestValue = 'default value'
 
server.on('request', function(req, res) {
  var pathName = req.url.split('/')[1];
  var pathToFile = path.join(__dirname + "/files/" + pathName);
  console.log("HERE", pathName)
  if(pathName == '') {
    console.log("GET /")
    return res.end("Hello World!")
  }

  if(req.method == "GET") {
    console.log("GET " + pathName)
    if(fs.existsSync(pathToFile)) {
      res.code = 200
      fs.readFile(pathToFile, 'utf8', function(err, data) {
        if(err) res.end(err);
        else res.end(data);
      })
    } else {
      res.code = 404;
      res.end('');
    }
  } else if (req.method == "POST") {
    console.log("POST " + pathName)
    var value = req.payload.toString()
    fs.writeFile(pathToFile, value, function(err) {
      if(err) console.log(err)
      res.end("Got it!")
    })
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