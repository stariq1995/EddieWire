import socket
import json
import sys
import time

PORT = 50000 + 2
BUFSIZE = 1024
DEC_SPACES = 7
TIME_STAMP_LENGTH = 11 + DEC_SPACES
def connect(host):
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	try:
		sock.connect((host, PORT))
	except socket.error:
		print "Could not initiate connection to " + host

	return sock

def put_timestamp(sender_no, buf, file_size):
	time_stamp = ('%.' + str(DEC_SPACES) + 'f') % time.time()
	buf[file_size + (TIME_STAMP_LENGTH * sender_no) : file_size + (TIME_STAMP_LENGTH * (sender_no + 1))] = time_stamp

def client(filename, chunk_size, sender_no, hops, host):
	with open(filename) as f:
		file_data = f.read()

	extra_data = b' ' * TIME_STAMP_LENGTH * (hops + 1)
	file_size = len(file_data)
	data = bytearray(file_data + extra_data)
	data_size = len(data)

	cmd_msg = {'total_size' : data_size, 'chunk_size' : chunk_size, 'sender_no' : sender_no, 'file_size' : file_size}
	client_sock = connect(host)
	client_sock.sendall(json.dumps(cmd_msg))
	reply = client_sock.recv(BUFSIZE)
	if not ('ok' in reply):
		print "Error communicating with next client"
		sys.exit(0)

	put_timestamp(sender_no, data, file_size)
	

	remaining_bytes = data_size
	while (remaining_bytes > 0):
		chunk = data[:chunk_size]
		data = data[chunk_size:]
		chunk_length = len(chunk)
		bytes_sent = 0
		while(bytes_sent < chunk_length):
			bytes_sent += client_sock.send(chunk)
			chunk = chunk[bytes_sent:]
		remaining_bytes -= bytes_sent


client('test_file.txt', 2, 0, 1, 'localhost')






