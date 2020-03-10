#!/usr/bin/env python

# WS client example

import asyncio
import websockets
import messages_pb2

async def hello():
	uri = "ws://128.97.92.77:3005"
	async with websockets.connect(uri) as websocket:

		wrapper = messages_pb2.WrapperMessage()

		# add CPU
		cpu_resource = wrapper.register_slave.slave.resources.add()
		cpu_resource.name = "cpu"
		cpu_resource.type = messages_pb2.Value.Type.SCALAR
		cpu_resource.scalar.value = 1

		# add MEMORY
		mem_resource = wrapper.register_slave.slave.resources.add()
		mem_resource.name = "mem"
		mem_resource.type = messages_pb2.Value.Type.SCALAR
		mem_resource.scalar.value = 1024

		print("Registering with master...")

		await websocket.send(wrapper.SerializeToString())

		while True:
			msg = await websocket.recv()
			wrapper = messages_pb2.WrapperMessage()
			wrapper.ParseFromString(msg)
			if wrapper.HasField('slave_registered'):
				print("Registered! ID:", wrapper.slave_registered.slave_id.value)
			else:
				print("Cannot parse message...")

asyncio.get_event_loop().run_until_complete(hello())