var coap        = require('coap')
  , server      = coap.createServer()

var fs = require('fs')
var path = require('path')

var latestValue = 'default value'
 
server.on('request', function(req, res) {
  var pathName = req.url.split('/')[1];
  var pathToFile = path.join(__dirname + "/files/" + pathName);
  console.log("HERE", pathName)
  console.log(req.method);

  if(pathName == '') {
    console.log("GET /")
    return res.end("Hello World!")
  }

  if(req.method == "GET") {
    console.log("GET " + pathName)
    if(fs.existsSync(pathToFile)) {
      res.code = 200
      res.setOption("Content-Format", "application/json");
      fs.readFile(pathToFile, 'utf8', function(err, data) {
        if(err) {
          console.log("Error reading file");
          res.end(err);
          return;
        }

        var r = [];
        var lines = data.split('\n')

        for (var i = 0; i < lines.length; i++) {
          var line = lines[i];
          if(line.length != 0) {
            var t = line.split(':')[0];
            var v = line.split(':')[1];
            r.push({'time':t,'value':v})
          }
        }


        //Only return the last 30 items to stay below the packet limit
        var f;
        if(r.length > 30) {
          f = r.slice(r.length-30);
        } else {
          f = r;
        }

        res.end(JSON.stringify(f));

      })
    } else {
      console.log("File doesn't exist");
      res.code = 404;
      res.end('');
    }
  } else if (req.method == "POST") {
    console.log("POST " + pathName)
    var v = Date.now().toString() + ':' + req.payload.toString() + '\n';
    fs.appendFile(pathToFile, v, function(err) {
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
