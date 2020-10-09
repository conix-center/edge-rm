var coap        = require('coap')
  , server      = coap.createServer()

var latestValue = ''
 
server.on('request', function(req, res) {
  var path = req.url.split('/')[1];
  console.log(path)
  if (path == 'post') {
    latestValue = req.payload.toString()
    res.end("Got it!")
  } else if (path == 'latest') {
    res.end(latestValue)
  } else {
    res.end('Hello World')
  }
})
 
const port = process.env.SERVER_PORT;
if (!port) {
  // the default CoAP port is 5683
  port = 5683
}

server.listen(port, function() {
  var req = coap.request('coap://localhost:3000/post')
 
  req.on('response', function(res) {
    res.pipe(process.stdout)
    res.on('end', function() {
      process.exit(0)
    })
  })
 
  req.end()
})