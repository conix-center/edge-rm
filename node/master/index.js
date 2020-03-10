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

var processWrapperMessage = function(msg) {
    if (msg.hasPong()) {
        console.log("PONG")
    } else if (msg.hasRegisterSlave()) {
        console.log("Register Slave!")
        var register = msg.getRegisterSlave()
        var slaveinfo = register.getSlave()
        var resources = slaveinfo.getResourcesList()
        for(var i=0; i<resources.length; i++) {
            console.log("Slave has" + resources[i].getName());
        }
    } else {
        console.log("Received unidentifiable message...")
    }
}

const wss = new WebSocket.Server({ server })

wss.on('connection', function connection(ws) {
    ws.on('message', function incoming(data) {
        // console.log('received: %s', data)
        // console.log("incoming message...")
        var msg = proto.WrapperMessage.deserializeBinary(Array.prototype.slice.call(data, 0))
        processWrapperMessage(msg)
    })

    var ping = new proto.PingSlaveMessage()
    ping.setConnected(true)
    var message = new proto.WrapperMessage()
    message.setPing(ping)

    // message.setValue('this is a slave id')

    var bytes = message.serializeBinary()
    ws.send(bytes)
})

server.listen(3005, function listening() {
    console.log('Listening on %d', server.address().port)
})
