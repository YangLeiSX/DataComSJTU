set terminal png size 960, 720
set output "lab5.png"
set xlabel "Time/s"
set ylabel "Congestion Window Size"
set title "Congestion Window of lab5.cc"
plot "lab5.cwnd" using 1:2 title "CWND" with lines lc rgb "black" , \
"lab5_drop.cwnd" using 1:2 title "Dropped" with points lt 1 pt 7 ps 5, \
"lab5_recv.cwnd" using 1:2 title "Received" with points  lt 2 pt 7 ps 2
exit