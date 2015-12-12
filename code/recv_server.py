#!/usr/bin/python

import socket
import json
import sys
import time
import timeit

PORT = 50000 + 2
BUFSIZE = 1024



def main():
	if len(sys.argv) < 3:
		usage()
	server(int(sys.argv[1]), int(sys.argv[2]), sys.argv[3])

def usage():
	print "Server: ./recv_server <%s> <%s> <%s>" % ("hops", "node number", "log file name")
	sys.exit(0)

def log(filename, line):
	with open(filename, "a") as f:
		f.write(line)

def server(hops, sender_no, log_file):
	while True:
		try:
			listen_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
			listen_socket.bind(('', PORT))
			listen_socket.listen(5)
			client_sock, addr = listen_socket.accept()
			handshake = client_sock.recv(BUFSIZE)
			info = json.loads(handshake)
			print info
			total_size = info['file_size']
			buf = bytearray(b' ' * total_size)
			client_sock.send('ok\n')
			read = 0
			to_read = total_size
			chunk_size = info['chunk_size']
			start_time = timeit.default_timer()
			while (read < to_read):
				view = memoryview(buf)
				read += client_sock.recv_into(view[read:], min(to_read - read, chunk_size))
			end_time = timeit.default_timer()

			with open('received_file%10d.datafile' % time.time(), 'w') as f:
				f.write(buf)

			log(log_file, str(end_time - start_time))
		except KeyboardInterrupt:
			print "Ctrl + c detected"
			sys.exit(0)

main()