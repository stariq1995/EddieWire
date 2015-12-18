#!/usr/bin/python

from socket import *
import sys

BEACON_PORT = 8000
BUFFER = 10
ESSID = "EdiNet"

def main():
	if len(sys.argv) < 2:
		usage()
	sock = socket(AF_INET, SOCK_DGRAM)
	sock.bind(('', BEACON_PORT))
	search(sys.argv[1], sock)


def usage():
	print "./scan_wifi %s" % ("Address")
	sys.exit(0)

def search(ip, sock):
	print "Searching for %s" % ip
	found = False
	while not found:
		essid, addr = sock.recvfrom(BUFFER)
		print "Heard ", essid, " from ", addr
		if ESSID in essid:
			host, port = addr
			if host == ip:
				found = True

main()