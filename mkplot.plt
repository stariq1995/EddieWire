reset
#set pointsize 2.0
set autoscale
#set xtics ("1MB" 1,"5MB" 5, "10MB" 10, "20MB" 20)
set xlabel "File Size (MB)"
set ylabel "Data Transfer Time (s)"
set key inside top center horizontal width -2 maxrows 1
#set grid
#set title "Data Transfer - Edison"
set terminal postscript eps color enhanced 'NimbusSanL-Regu' 14
set size 0.6,0.5
set output "Edison-dataTransfer.eps"
set yrange [:150]
plot 	"file_size_delay.dat" using 1:2 title col with linespoints, \
 	"file_size_delay.dat" using 1:3 title col with linespoints
set yrange [:10000]
set logscale y
set output "Edison-dataTransfer_logscale.eps"
replot
