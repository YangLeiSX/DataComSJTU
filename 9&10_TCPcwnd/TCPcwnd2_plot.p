set terminal png size 960, 720
set output "lab6.png"
set xlabel "Time/s"
set ylabel "Congestion Window Size"
set title "Congestion Window of lab6.cc"
plot "lab6.cwnd" using 1:2 title "CWND" with lines lc rgb "black" lw 2 , \
"lab6_drop.cwnd" using 1:2 title "Dropped" with points lt 1 pt 7 ps 5, \
"lab6_recv.cwnd" using 1:2 title "Received" with points  lt 2 pt 7 ps 2
exit