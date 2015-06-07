

# Opening first input file
log_filename = raw_input("Log filename:")
log = open(log_filename)
log_lines = log.readlines()

# Opening second input file
cmp_filename = raw_input("Cmp filename:")
cmp = open(cmp_filename)
cmp_lines = cmp.readlines()

# Opening output file
out_filename = raw_input("Output filename:")
out = open(out_filename, 'w')

size = 0
count_recv = 0
count_correct = 0
sum = 0.0

# Processing input according to size
for i in range(len(log_lines)):
  l_entries = log_lines[i].split()
  # New file class
  if int(l_entries[1]) != size:
    # print size, count_recv, sum
    # avoid writing the first time
    if count_correct > 0:
      print size, count_recv, count_correct, sum
      avg = sum / (count_correct*1.0)
      s = str(size) + " " + str(avg) + " " + str(count_recv*10) + " " + str(count_correct*10) + "\n"
      out.write(s)
    count_recv = 0
    count_correct = 0
    sum = 0
    size = int(l_entries[1])
    # process first line
    count_recv += 1
    c_entries = cmp_lines[i].split()
    # correct file
    if int(c_entries[1]) == 0:
      count_correct += 1
      sum += int(float(l_entries[2]))
  # Old file class
  else:
    count_recv += 1
    c_entries = cmp_lines[i].split()
    # correct file
    if int(c_entries[1]) == 0:
      count_correct += 1
      sum += int(float(l_entries[2]))

# Inserting last entry
if count_correct > 0:
  print size, count_recv, count_correct, sum
  avg = sum / (count_correct*1.0)
  s = str(size) + " " + str(avg) + " " + str(count_recv) + " " + str(count_correct) + "\n"
  out.write(s)


print "Done processing ", len(log_lines), " logs"


