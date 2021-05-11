const https = require('https')



function reduce(data) {

	const msg = JSON.stringify({
		content: data.toString()
	})

	const options = {
	  hostname: 'discord.com',
	  port: 443,
	  path: '/api/webhooks/841802774345547797/6vqvttgl4jGad9QyR941XlPldw_8PVr6oppQhFPCi1s19SMreBu39OcCKBoAYOx-hRYg',
	  method: 'POST',
	  headers: {
	    'Content-Type': 'application/json',
	    'Content-Length': msg.length
	  }
	}

	const req = https.request(options, res => {
	  console.log(`statusCode: ${res.statusCode}`)

	  res.on('data', d => {
	    process.stdout.write(d)
	  })
	})

	req.on('error', error => {
	  console.error(error)
	})

	req.write(msg)
	req.end()

	console.log(data);
}

module.exports = {reduce}