gnuplot
set terminal png size 640,480
set output "lab4.png"
set xlabel "x-axis"
set ylabel "y-axis"
set title "Wifi Stations Location Trace"
plot "lab4_5.dat" using 1:2 title "station n5" with linespoints, "lab4_6.dat" using 1:2 title "station n6" with linespoints, "lab4_7.dat" using 1:2 title "station n7" with linespoints
exit