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
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

static void
MyPacketTrace ( Ptr< const Packet > packet )
{
  std::cout << "My traced packet size:" << packet->GetSize() << std::endl << std::endl;
}

static void
MyPacketTraceWithContext ( std::string context, Ptr< const Packet> packet)
{
  std::cout << context << "My traced packet size: " << packet->GetSize() << std::endl;
}

int
main (int argc, char *argv[])
{

  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  Time::SetResolution (Time::NS);
  // LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  // LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  
  // crate nodes
  NodeContainer nodes;
  nodes.Create (2);
  // setup channel
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  // install devices
  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);
  // install internet stack 
  InternetStackHelper stack;
  stack.Install (nodes);
  // assign ip address
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);
  // setup server
  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));
  // setup client
  UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (3));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));
  // setup trace
  // AsciiTraceHelper ascii;
  // pointToPoint.EnableAsciiAll(ascii.CreateFileStream("myfirst.tr"));
  // pointToPoint.EnablePcapAll("myfirst");
  // setup local trace
  Config::ConnectWithoutContext(
    "/NodeList/1/DeviceList/0/$ns3::PointToPointNetDevice/MacTx",
    MakeCallback(&MyPacketTrace));
  Config::Connect(
    "/NodeList/1/DeviceList/0/$ns3::PointToPointNetDevice/MacTx",
    MakeCallback(&MyPacketTraceWithContext));
  // run simulation
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
