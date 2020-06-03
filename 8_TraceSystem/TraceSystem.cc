/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/vector.h"
// Default Network Topology
//
//   Wifi 10.1.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |    10.1.1.0
// n5   n6   n7   n0 -------------- n1   n2   n3   n4
//                   point-to-point  |    |    |    |
//                                   ================
//                                     LAN 10.1.2.0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Lab4");

// static void
// LocTrace ( Ptr<const MobilityModel> model)
// {
 
//   std::cout << "Station moved to: x " << model->GetPosition().x << " \ty " << model->GetPosition().y << std::endl;
// }

static void
LocTraceWithContext ( std::string context, Ptr<const MobilityModel> model)
{
 
  std::cout << "Station " << context <<" moved to: x " << model->GetPosition().x << " \ty " << model->GetPosition().y << std::endl;
}

static void
LocRecord (Ptr<OutputStreamWrapper> stream, Ptr<const MobilityModel> model)
{
  *stream->GetStream() << model->GetPosition().x << "\t" << model->GetPosition().y << std::endl;
}

int 
main (int argc, char *argv[])
{
  bool verbose = false;
  uint32_t nCsma = 3;
  uint32_t nWifi = 3;
  bool tracing = false;
  // parse command line parameters
  CommandLine cmd;
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);

  cmd.Parse (argc,argv);

  if (nWifi > 18)
    {
      std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box" << std::endl;
      return 1;
    }
  // setup log level
  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }
  // setup p2p topology
  // p2p nodes
  NodeContainer p2pNodes;
  p2pNodes.Create (2);
  // p2p channel
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  // p2p devices
  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);
  
  // setup LAN topology
  // csma nodes
  NodeContainer csmaNodes;
  csmaNodes.Add (p2pNodes.Get (1));
  csmaNodes.Create (nCsma);
  // csma channel
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
  // csma devices
  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);
  
  // wifi topology
  // wifi nodes
  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);
  NodeContainer wifiApNode = p2pNodes.Get (0);
  // wifi channel
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());
  // setup wifi network
  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
  
  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);
  // setup station mobility
  MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  mobility.Install (wifiStaNodes);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);
  // install internet stack
  InternetStackHelper stack;
  stack.Install (csmaNodes);
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);
  // assign ip address
  Ipv4AddressHelper address;

  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);

  address.SetBase ("10.1.3.0", "255.255.255.0");
  address.Assign (staDevices);
  address.Assign (apDevices);
  // setup server
  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (nCsma));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));
  // setup client
  UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (nCsma), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = 
    echoClient.Install (wifiStaNodes.Get (nWifi - 1));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));
  // setup route
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (10.0));

  if (tracing == true)
    {
      pointToPoint.EnablePcapAll ("third");
      phy.EnablePcap ("third", apDevices.Get (0));
      csma.EnablePcap ("third", csmaDevices.Get (0), true);
    }

  // trace with context
  Config::Connect(
    "/NodeList/5/$ns3::MobilityModel/CourseChange",
    MakeCallback(&LocTraceWithContext));
  Config::Connect(
    "/NodeList/6/$ns3::MobilityModel/CourseChange",
    MakeCallback(&LocTraceWithContext));
  Config::Connect(
    "/NodeList/7/$ns3::MobilityModel/CourseChange",
    MakeCallback(&LocTraceWithContext));

  // trace without context
  // Config::ConnectWithoutContext(
  //   "/NodeList/5/$ns3::MobilityModel/CourseChange",
  //   MakeCallback(&LocTrace));
  // Config::ConnectWithoutContext(
  //   "/NodeList/6/$ns3::MobilityModel/CourseChange",
  //   MakeCallback(&LocTrace));
  // Config::ConnectWithoutContext(
  //   "/NodeList/7/$ns3::MobilityModel/CourseChange",
  //   MakeCallback(&LocTrace));
  // trace file
  std::string filename5 = "lab4_5.dat";
  std::string filename6 = "lab4_6.dat";
  std::string filename7 = "lab4_7.dat";
  std::ios::openmode filemode = std::ios::out;
  Ptr<OutputStreamWrapper> stream5 = Create<OutputStreamWrapper>(filename5, filemode);
  Config::ConnectWithoutContext (
    "/NodeList/5/$ns3::MobilityModel/CourseChange", 
    MakeBoundCallback(&LocRecord, stream5));
  Ptr<OutputStreamWrapper> stream6 = Create<OutputStreamWrapper>(filename6, filemode);
  Config::ConnectWithoutContext (
    "/NodeList/6/$ns3::MobilityModel/CourseChange", 
    MakeBoundCallback(&LocRecord, stream6));
  Ptr<OutputStreamWrapper> stream7 = Create<OutputStreamWrapper>(filename7, filemode);
  Config::ConnectWithoutContext (
    "/NodeList/7/$ns3::MobilityModel/CourseChange", 
    MakeBoundCallback(&LocRecord, stream7));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
