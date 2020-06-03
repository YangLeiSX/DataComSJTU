# README

实验名称: Lab3 Building Wireless topology

实验内容: 

1. In this section we are going to further expand our knowledge of ns-3 network devices and channels to cover an example of a wireless network.即构建一个WLAN网络、P2P网络和LAN（CSMA）网络的组合，实现如下的拓扑。

   // Wifi 10.1.3.0   AP

   // *     *      *       *

   // |       |       |       |                 10.1.1.0

   // n5  n6  n7   n0 -------------------------- n1   n2   n3   n4

   //                                    point-to-point           |.       |      |      |

   //                                                                        \=\=\=\=\=\=\=\=\=\=

   //                                                                           LAN 10.1.2.0

2. Exercise: Create three access point A, B and C. Wire (Connect) A-B, B-C using point to point link (100Mbps, 2ms).即构建两个P2P网络和两个WI-FI网络，实现如下的拓扑。

   // Wifi 10.1.3.0  AP

   // *     *    *      *

   // |      |      |      |          10.1.1.0                 10.1.2.0

   // n3  n4  n5  n0 -------------- n1 --------------- n2  n6  n7  n8

   //                                    P2P                      P2P            |    |      |      |

   //                              100Mbps              100Mbps      *    *    *     *

   //                                   2ms                     2ms           AP Wifi 10.1.4.0