import subprocess

def found():
	proc = subprocess.Popen('iwlist wlan0 scan essid EdiNet | grep "EdiNet"', stdout=subprocess.PIPE, shell=True, )
	out = proc.communicate()[0]
	count = len(out.split('\n')) - 1
	print "Found %d nodes" % count
	if count == 4:
		return True
	return False

def main():
	b = False
	b = found()
	while not b:
		b = found()

main()