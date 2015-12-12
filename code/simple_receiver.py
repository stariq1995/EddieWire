import socket
import json
import sys
import time

PORT = 50000 + 2
BUFSIZE = 1024
DEC_SPACES = 7
TIME_STAMP_LENGTH = 11 + DEC_SPACES

def put_timestamp(sender_no, buf, file_size):
	time_stamp = ('%.' + str(DEC_SPACES) + 'f') % time.time()
	print len(buf)
	print file_size + (TIME_STAMP_LENGTH * sender_no)
	print file_size + (TIME_STAMP_LENGTH * (sender_no + 1))

	buf[file_size + (TIME_STAMP_LENGTH * sender_no) : file_size + (TIME_STAMP_LENGTH * (sender_no + 1))] = time_stamp

def server(hops, sender_no):
	listen_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	listen_socket.bind(('', PORT))
	listen_socket.listen(5)
	client_sock, addr = listen_socket.accept()
	handshake = client_sock.recv(BUFSIZE)
	info = json.loads(handshake)
	print info
	total_size = info['total_size']
	buf = bytearray(b' ' * total_size)
	client_sock.send('ok\n')
	read = 0
	to_read = total_size
	while (read < to_read):
		chunk_size = info['chunk_size']
		read_chunk = 0
		remaining_chunk = min(to_read, chunk_size)
		while (read_chunk < remaining_chunk):
			view = memoryview(buf)
			read_chunk += client_sock.recv_into(view[read + read_chunk:], remaining_chunk - read_chunk)
		read += read_chunk

	put_timestamp(sender_no, buf, info['file_size'])
	with open('received_file%10d.txt' % time.time(), 'w') as f:
		f.write(buf)

server(1, 1)

