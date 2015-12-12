#!/usr/bin/python

import socket
import json
import sys
import time

PORT = 50000 + 2
BUFSIZE = 1024


def main():
	if len(sys.argv) < 6:
		usage()
	client(sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4]), sys.argv[5])

def usage():
	print "Server: ./recv_server <%s> <%s> <%s> <%s> <%s>" % ("filename", "chunk_size", "hops", "node number", "host")
	sys.exit(0)

def connect(host):
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	try:
		sock.connect((host, PORT))
		return sock
	except socket.error:
		print "Could not initiate connection to " + host

	

def client(filename, chunk_size, hops, sender_no, host):
	with open(filename) as f:
		file_data = f.read()

	file_size = len(file_data)
	data = bytearray(file_data)

	cmd_msg = {'chunk_size' : chunk_size, 'sender_no' : sender_no, 'file_size' : file_size}
	client_sock = connect(host)
	client_sock.sendall(json.dumps(cmd_msg))
	reply = client_sock.recv(BUFSIZE)
	if not ('ok' in reply):
		print "Error communicating with next client"
		sys.exit(0)
	

	remaining_bytes = file_size

	while (remaining_bytes > 0):
		chunk = data[:chunk_size]
		data = data[chunk_size:]
		chunk_length = len(chunk)
		bytes_sent = 0
		while(bytes_sent < chunk_length):
			bytes_sent += client_sock.send(chunk)
			chunk = chunk[bytes_sent:]
		remaining_bytes -= bytes_sent

	client_sock.close()


main()





