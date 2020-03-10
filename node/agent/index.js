const Messages = require('./messages_pb')
const WebSocket = require('ws')

process.env.NODE_TLS_REJECT_UNAUTHORIZED = '0'

const ws = new WebSocket('wss://localhost:3005/', { origin: 'https://localhost:3005' })

ws.on('open', function open() {
    // console.log('connected')
    console.log('registering with master...')

    //create slave info protobuf
    var register = new proto.RegisterSlaveMessage()
    var slaveinfo = new proto.SlaveInfo()
    var cpuResource = new proto.Resource()
    cpuResource.setName("cpu")
    cpuResource.setType(proto.Value.Type.Scalar)
    var cpuResourceScalar = new proto.Value.Scalar()
    cpuResourceScalar.setValue("1")
    cpuResource.setScalar(cpuResourceScalar)
    slaveinfo.addResources(cpuResource)
    register.setSlave(slaveinfo)

    //create wrapper message
    var message = new proto.WrapperMessage()
    message.setRegisterSlave(register)
    ws.send(message.serializeBinary())
})

ws.on('close', function close() {
    // console.log('disconnected')
})

ws.on('message', function incoming(data, flags) {
    // console.log('message')
    var bytes = Array.prototype.slice.call(data, 0)
    var message = proto.WrapperMessage.deserializeBinary(bytes)

    if(message.hasPing()) {
        //Ping message received
    	console.log("PING")

    	var pong = new proto.PongSlaveMessage()
	    var message = new proto.WrapperMessage()
	    message.setPong(pong)
	    ws.send(message.serializeBinary())
    } else if(message.hasSlaveRegistered()) {
        //Slave registered message received
        console.log("Registered!")
        var slaveid = message.getSlaveRegistered().getSlaveId()
        console.log("Slave ID:", slaveid)
    }

    // ws.close()
})