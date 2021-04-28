var coap        = require('coap');
var fs = require('fs');

if(process.argv.length < 6) {
  console.log("USAGE: node scheduler.js ./path/to/reduce.js KEY HOST PORT")
  return
}

try {
  var testFile = fs.readFileSync(process.argv[2]);
  var key = process.argv[3]
  var host = process.argv[4]
  var port = process.argv[5]

  var req = coap.request({
    host: host,
    // host: 'conixdb.com',
    port: port,
    pathname: `/reduce/${key}/create`,
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
      pathname: `/reduce/${key}/data`,
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
} catch(e) {
  console.log("ERROR")
}


