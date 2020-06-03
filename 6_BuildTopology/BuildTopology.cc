/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

// Default Network Topology
//
//      192.168.10.0        192.168.50.0
// n0 ---------------- n1 ---------------- n2
//     point-to-point      point-to-point
//       5Mbps 2ms           1Mbps 2ms  
//        

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Lab2Script");

int 
main (int argc, char *argv[])
{
  // Parse command line parameters 
  CommandLine cmd;
  cmd.Parse (argc,argv);
  // Enbale log 
  LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  // Create nodes
  NodeContainer p2p1Nodes;
  p2p1Nodes.Create (2);

  NodeContainer p2p2Nodes;
  p2p2Nodes.Add(p2p1Nodes.Get(1));  
  p2p2Nodes.Create (1);
  // Setup p2p channel
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer p2p1Devices;
  p2p1Devices = pointToPoint.Install (p2p1Nodes);

  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer p2p2Devices;
  p2p2Devices = pointToPoint.Install (p2p2Nodes);
  // Install Internet stack
  InternetStackHelper stack;
  stack.Install (p2p1Nodes.Get (0));
  stack.Install (p2p2Nodes);
  // Setup IP address for devices
  Ipv4AddressHelper address1;
  address1.SetBase ("192.168.10.0", "255.255.255.0");
  Ipv4InterfaceContainer p2p1Interfaces;
  p2p1Interfaces = address1.Assign (p2p1Devices);

  Ipv4AddressHelper address2;
  address2.SetBase ("192.168.50.0", "255.255.255.0");
  Ipv4InterfaceContainer p2p2Interfaces;
  p2p2Interfaces = address2.Assign (p2p2Devices);
  // Setup OnOff Server
  Address remoteAddress (InetSocketAddress (p2p2Interfaces.GetAddress(1), 9014));
  OnOffHelper onOffHelper ("ns3::TcpSocketFactory", remoteAddress);               // Setup target address and port
  onOffHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  onOffHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  onOffHelper.SetAttribute("DataRate", StringValue("50000bps"));
  onOffHelper.SetAttribute("PacketSize", StringValue("512"));
  onOffHelper.SetAttribute("MaxBytes", StringValue("2000"));
  // Deploy the application and run
  ApplicationContainer serverApp = onOffHelper.Install (p2p1Nodes.Get (0));
  serverApp.Start (Seconds (1.0));
  serverApp.Stop (Seconds (3.0));

  // Setup SinkPacket Server
  Address LocalAddress (InetSocketAddress (p2p2Interfaces.GetAddress(1), 9014));  // Setup listen address and port
  PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", LocalAddress);
  // Deploy the application and run
  ApplicationContainer clientApps = packetSinkHelper.Install (p2p2Nodes.Get (1));
  clientApps.Start (Seconds (1.0));
  clientApps.Stop (Seconds (3.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  // Trace the packets
  pointToPoint.EnablePcapAll ("lab2");

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
