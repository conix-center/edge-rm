var coap        = require('coap');
var fs = require('fs');

var testFile = fs.readFileSync('./test.js') 

setTimeout(function() {
  // var req2 = coap.request('coap://conixdb.com:3002/latest')
  var req2 = coap.request({
    host: '128.97.92.77',
    port: 3004,
    pathname: '/reduce/a8df95627c1b71fbd016/data',
    method: "POST"
  });

  req2.on('response', function(res) {
    // console.log(JSON.parse(String(res.payload)));
    console.log(String(res.payload));
    res.on('end', function() {
      process.exit(0)
    })
  })

  req2.write("Hello!")

  req2.end()
}, 2000)
