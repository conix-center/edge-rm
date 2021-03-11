var coap        = require('coap');

var req = coap.request({
  host: 'localhost',
  // host: 'conixdb.com',
  port: 3002,
  pathname: '/test',
  method: "POST"
})
// var req = coap.request('coap://conixdb.com:3002/post')

req.on('response', function(res) {
  //res.pipe(process.stdout)
  res.on('end', function() {
    // process.exit(0)
  })
})

req.write('1')
req.end()

setTimeout(function() {
  // var req2 = coap.request('coap://conixdb.com:3002/latest')
  var req2 = coap.request({
    host: 'localhost',
    port: 3002,
    pathname: '/test',
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
