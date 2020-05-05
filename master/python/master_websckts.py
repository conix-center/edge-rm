#!/usr/bin/env python

# WS server example
import sys
import asyncio
import websockets
import argparse

import messages_pb2

import db

sockets = dict()

async def register_slave(websocket, wrapper):
	print("Registering slave...")

	# build agent info
	resources = []
	for resource in wrapper.register_slave.slave.resources:
		resources.append([resource.name, resource.scalar.value])

	# add agent to db
	agent_id = db.add_agent(resources, "webs")
	sockets[websocket] = agent_id
	print(db.get_all())

	# send response message
	response = messages_pb2.WrapperMessage()
	response.slave_registered.slave_id.value = str(agent_id)
	await websocket.send(response.SerializeToString())

async def unregister_slave(websocket):
	agent_id = sockets[websocket]
	db.delete_agent(agent_id)
	del sockets[websocket]
	print("Slave disconnected...")

async def entry(websocket, path):
	try:
		while True:
			msg = await websocket.recv()

			wrapper = messages_pb2.WrapperMessage()
			wrapper.ParseFromString(msg)

			if wrapper.HasField('register_slave'):
				await register_slave(websocket, wrapper)
			else:
				print("Unsupported message...")
	except websockets.exceptions.ConnectionClosedError:
		pass
	finally:
		await unregister_slave(websocket)

def main(host, port):  # pragma: no cover
	if sys.version_info[0] < 3:
		raise Exception("Must be using Python 3")
	db.refresh_db()
	loop = asyncio.new_event_loop()
	start_server = websockets.serve(entry, host=host, port=port, loop=loop)
	loop.run_until_complete(start_server)
	loop.run_forever()

if __name__ == "__main__":  # pragma: no cover
	parser = argparse.ArgumentParser(description='Launch the Websockets Resource Manager Master')
	parser.add_argument('--host', required=True, help='the LAN IP to bind to.')
	parser.add_argument('--port', required=True, help='the local machine port to bind to.')
	args = parser.parse_args()
	main(args.host, args.port)