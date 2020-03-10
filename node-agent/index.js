const Messages = require('./messages_pb')
const WebSocket = require('ws')

process.env.NODE_TLS_REJECT_UNAUTHORIZED = '0'

const ws = new WebSocket('wss://localhost:8080/', { origin: 'https://localhost:8080' })

ws.on('open', function open() {
    console.log('connected')
})

ws.on('close', function close() {
    console.log('disconnected')
})

ws.on('message', function incoming(data, flags) {
    console.log('message')
    var bytes = Array.prototype.slice.call(data, 0)
    var message = proto.WrapperMessage.deserializeBinary(bytes)

    if(message.hasPing()) {
    	console.log("Ping", message.getPing().getConnected())

    	var pong = new proto.PongSlaveMessage()
	    var message = new proto.WrapperMessage()
	    message.setPong(pong)
	    ws.send(message.serializeBinary())
    }

    ws.close()
})