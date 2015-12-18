import subprocess

def found():
	proc = subprocess.Popen('sudo iwlist wlan0 scan essid EdiNet | grep "EdiNet"', stdout=subprocess.PIPE, shell=True, )
	out = proc.communicate()[0]
	if len(out.split('\n')) - 1 == 4:
		return True
	return False

def main():
	b = False
	b = found()
	while not b:
		b = found()

main()