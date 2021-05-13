var coap        = require('coap');
var fs = require('fs');

var testFile = fs.readFileSync('./test.js') 

var req = coap.request({
  host: 'localhost',
  // host: 'conixdb.com',
  port: 5683,
  pathname: '/reduce/123/create',
  method: "POST"
})
// var req = coap.request('coap://conixdb.com:3002/post')

req.on('response', function(res) {
  console.log(String(res.payload));
  res.on('end', function() {
    // process.exit(0)
  })
})

req.write(testFile)
req.end()

setTimeout(function() {
  // var req2 = coap.request('coap://conixdb.com:3002/latest')
  var req2 = coap.request({
    host: 'localhost',
    port: 5683,
    pathname: '/reduce/123/data',
    method: "POST"
  });

  req2.on('response', function(res) {
    // console.log(JSON.parse(String(res.payload)));
    console.log(String(res.payload));
    res.on('end', function() {
      process.exit(0)
    })
  })

  req2.write("Hello 2!")

  req2.end()

  setTimeout(function() {
    var req2 = coap.request({
      host: 'localhost',
      port: 5683,
      pathname: '/reduce/123/data',
      method: "GET"
    });

    req2.on('response', function(res) {
      console.log(JSON.parse(String(res.payload)));
      res.on('end', function() {
        process.exit(0)
      })
    })

    req2.end()
  }, 2000)
}, 2000)
