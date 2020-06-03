
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/packet-sink.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Lab5");

// ===========================================================================
//
//         node 0                 node 1
//   +----------------+    +----------------+
//   |    ns-3 TCP    |    |    ns-3 TCP    |
//   +----------------+    +----------------+
//   |    10.1.1.1    |    |    10.1.1.2    |
//   +----------------+    +----------------+
//   | point-to-point |    | point-to-point |
//   +----------------+    +----------------+
//           |                     |
//           +---------------------+
//                5 Mbps, 2 ms
//
// ===========================================================================
//

AsciiTraceHelper asciiTraceHelper;

static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldCwnd << "\t" << newCwnd << std::endl;
}

static void 
TraceCwndChange(std::string traceName)
{
  Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (traceName.c_str());
  Config::ConnectWithoutContext (
    "/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", 
    MakeBoundCallback (&CwndChange, stream));
}

static void
RxDrop (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> p)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t0" << std::endl;
}

static void
TraceDrop(std::string dropName)
{
  Ptr<OutputStreamWrapper> DropFile = asciiTraceHelper.CreateFileStream (dropName.c_str());
  Config::ConnectWithoutContext (
    "/NodeList/1/DeviceList/0/$ns3::PointToPointNetDevice/PhyRxDrop",
    MakeBoundCallback (&RxDrop, DropFile));
}

static void
RxEnd (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> p)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t0" << std::endl;
}

static void
TraceRecv(std::string recvName)
{
  Ptr<OutputStreamWrapper> RecvFile = asciiTraceHelper.CreateFileStream (recvName);
  Config::ConnectWithoutContext (
    "/NodeList/1/DeviceList/0/$ns3::PointToPointNetDevice/PhyRxEnd",
    MakeBoundCallback (&RxEnd, RecvFile));
}

int
main (int argc, char *argv[])
{
  // parse commnad line parameters
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  // create nodes
  NodeContainer nodes;
  nodes.Create (2);
  
  // create channel 
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  // create nodes
  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

  // setup error model
  Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
  em->SetAttribute ("ErrorRate", DoubleValue (0.00001));
  devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
  
  // install internet stack
  InternetStackHelper stack;
  stack.Install (nodes);

  // assign address
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  // setup sink application
  uint16_t sinkPort = 9;
  Address sinkAddress (InetSocketAddress (interfaces.GetAddress (1), sinkPort));
  PacketSinkHelper packetSinkHelper (
    "ns3::TcpSocketFactory", 
    InetSocketAddress (interfaces.GetAddress(1), sinkPort));
  ApplicationContainer sinkApps = packetSinkHelper.Install (nodes.Get (1));
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (10.));
  
  // setup bulk send application
  BulkSendHelper bulkSendHelper (
    "ns3::TcpSocketFactory", 
    InetSocketAddress (interfaces.GetAddress(1), sinkPort));
  bulkSendHelper.SetAttribute ("SendSize", UintegerValue (1040));
  bulkSendHelper.SetAttribute ("MaxBytes", UintegerValue (1040 * 1000));
  ApplicationContainer sourceApps = bulkSendHelper.Install (nodes.Get (0));
  sourceApps.Start (Seconds (0.));
  sourceApps.Stop (Seconds (10.));

  Simulator::Schedule (Seconds (0.00001), &TraceCwndChange, std::string("lab6.cwnd"));
  Simulator::Schedule (Seconds (0.00002), &TraceDrop, std::string("lab6_drop.cwnd"));
  Simulator::Schedule (Seconds (0.00003), &TraceRecv, std::string("lab6_recv.cwnd"));

  // begin simulation
  Simulator::Stop (Seconds (10));
  Simulator::Run ();

  Simulator::Destroy ();

  return 0;
}

