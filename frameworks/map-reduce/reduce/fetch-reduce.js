var coap        = require('coap');
var path        = require('path');
var fs = require('fs');

if(process.argv.length < 6) {
  console.log("USAGE: node fetch-reduce.js ./path/to/output.txt KEY HOST PORT")
  return
}

let fileDirectory = path.join(process.cwd(), process.argv[2])

try {
  var key = process.argv[3]
  var host = process.argv[4]
  var port = process.argv[5]

  var req2 = coap.request({
    host: host,
    port: port,
    pathname: `/reduce/${key}/data`,
    method: "GET"
  });

  req2.on('response', function(res) {
    var response = JSON.parse(String(res.payload));
    console.log(response)
    if (response.length <= 0) return;

    var stream = fs.createWriteStream(fileDirectory, {flags:'a'});
    stream.end(response.join('\n') + "\n");

    res.on('end', function() {
      process.exit(0)
    })
  })

  req2.end()
  
} catch(e) {
  console.log("ERROR")
  console.log(e)
}


