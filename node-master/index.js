const Messages = require('./messages_pb')

const Express = require('express')
const Https = require('https')
const WebSocket = require('ws')
const FileSystem = require('fs')
const app = Express()

var slaves = []

app.use(function (req, res) {
    res.send({ msg: 'hello' })
})

const server = Https.createServer({
    key: FileSystem.readFileSync('key.pem'),
    cert: FileSystem.readFileSync('cert.pem'),
    passphrase: 'protobufjstest'
}, app)

const wss = new WebSocket.Server({ server })

wss.on('connection', function connection(ws) {
    ws.on('message', function incoming(message) {
        console.log('received: %s', message)
    })

    var ping = new proto.PingSlaveMessage()
    var message = new proto.WrapperMessage()
    message.setPing(ping)

    // message.setValue('this is a slave id')

    var bytes = message.serializeBinary()
    ws.send(bytes)
})

server.listen(8080, function listening() {
    console.log('Listening on %d', server.address().port)
})
