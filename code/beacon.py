import subprocess as sp
import time
from socket import *

BEACON_INTERVAL = 1
BEACON_PORT = 8000
ESSID = "EdiNet"
ip_list = ['192.168.1.1', '192.168.1.10', '192.168.1.11', '192.168.1.12']


def getIP():
	proc = sp.Popen('ifconfig wlan0 | grep "inet addr"')
	out = proc.communicate()[0]
	ip = out.split()[0].split(':')[1]
	return ip

def beacon(ips, my_ip):
	print "Beaconing to ", ips ," from ", my_ip
	sock = socket(AF_INET, SOCK_DGRAM)
	sock.bind(('', 0))
	while True:
		for ip in ips:
			addr = ip, BEACON_PORT
			sock.sendto(ESSID, addr)
			time.sleep(BEACON_INTERVAL)

def main():
	my_ip = getIP()
	ips = [ip for ip in ip_list if ip != my_ip]
	beacon(ips, my_ip)


main()