

with open("2-hop-1008-chunk.dat") as f:
	raw = f.readlines()
	raw = [line.strip().split()[1:] for line in raw]
	raw = [[int(size)/(1024.0 * 1024.0), float(time)/(1000000.0)] for [size, time] in raw]

d = {}
for [size, time] in raw:
	timel = d.get(size, [])
	timel.append(time)
	d[size] = timel


for k in d.keys():
	d[k] = round(sum(d[k])/len(d[k]), 4)

print d