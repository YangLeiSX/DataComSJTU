# README

实验名称：Lab5 and Lab6: TCP Congestion Window 

实验内容：

1. Lab5.cc: Produce a figure showing TCP congestion window size, dropped packets and received packets. Use the same network configuration as in mysixth.cc. 即记录TCP发送窗口的变化并绘图，同时在图中绘制报文发送成功和失败的时间。

2. Lab6.cc: Remember there are two approaches to connect your trace sink to the CongestionWindow trace source. 

   a)     Method 1: using SocketObject->TraceConnect(……) 

   b)     Method 2: using Config::Connect(……  ) 

   We have used method 1 in lab5, use method 2 in lab6.cc to get the value of TCP congestion window. 即将lab5中的代码换用Config的连接方法，记录发送窗口的变化和发送成功与失败的时间。