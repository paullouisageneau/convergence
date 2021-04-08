#!/usr/bin/env python3

import asyncio
import websockets
import struct
import binascii

port = 8080

peers = {}

async def handle(websocket, path):
	global peers
	peerId = None
	try:
		print('Incoming')
		while True:
			message = await websocket.recv()
			header = message[:8]
			source = message[8:24]
			destination = message[24:40]
			payload = message[40:]
			type, size = struct.unpack('>II', header);
			print('Message {} from {}'.format(type, binascii.hexlify(source)))

			if type == 1:
				peerId = source
				peers[peerId] = websocket;
				peerList = bytearray()
				print('Total {} peers'.format(len(peers)))
				for peer in peers:
					if peer != source:
						peerList.extend(peer)

				await websocket.send(struct.pack('>II', 2, len(peerList)) + bytearray(16) + source + peerList);
			else:
				if destination in peers:
					await peers[destination].send(message)
	except Exception as e:
		print(e)
		if peerId:
			del peers[peerId]


start_server = websockets.serve(handle, 'localhost', port)

print('Listening on port {}'.format(port));
asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
