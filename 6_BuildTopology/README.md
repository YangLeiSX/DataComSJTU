# README

实验名称:Lab2 Building Topologies 

实验内容: 

1. In this section we are going to add a bus network to the point-to-point network we built in myfirst.cc, so the network topology will look like the following.即构建一个 point 到 pooint 网络和 LAN(CSMA)网络 的组合，实现如下的拓扑。 

   ​                    10.1.1.0
    n0 ------------------------ n1   n2    n3    n4 

   ​             point-to-point         ｜    ｜    ｜    ｜

   ​                                               \=\=\=\=\=\=\=\=\=\=\=\=

   ​                                                   LAN 10.1.2.0 

2. Builda2-hopPoint-to-Pointnetworkasillustratedbelow.即构建两个point到point的网络，实现如下的拓扑。 

   ​                 192.168.10.0                           192.168.50.0
    n0 ------------------------ n1-------------------------------- n2 

   ​              point-to-point                         point-to-point 

   ​                5Mbps, 2ms                             1Mbps, 2ms 

   