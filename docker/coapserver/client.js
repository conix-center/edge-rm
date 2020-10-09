var coap        = require('coap');

var req = coap.request('coap://conixdb.com:3002/post')

req.on('response', function(res) {
  res.pipe(process.stdout)
  res.on('end', function() {
    // process.exit(0)
  })
})

req.write('TESTING')

req.end()

setTimeout(function() {
  var req2 = coap.request('coap://conixdb.com:3002/latest')
  req2.on('response', function(res) {
    res.pipe(process.stdout)
    res.on('end', function() {
      process.exit(0)
    })
  })
  req2.end()
}, 2000)