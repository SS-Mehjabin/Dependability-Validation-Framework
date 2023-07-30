/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 Sri Lanka Institute of Information Technology
 *
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
 *
 * Author: Jude Niroshan <jude.niroshan11@gmail.com>
 */

#include <iostream>
#include <cmath>
#include "ns3/aodv-module.h"
#include "ns3/netanim-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h" 
#include "ns3/v4ping-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/csma-helper.h"
#include "ns3/trust-aodv-module.h"
#include "ns3/trust-manager-helper.h"

using namespace ns3;
using namespace aodv;

/**
 * \ingroup trust-aodv-examples
 * \ingroup examples
 * \brief Test script.
 * 
 * This is non-linear simulation of 7 nodes with static positions
 * which uses a malicious node and trust based routing protocol
 */
class AodvExample 
{
public:
  AodvExample ();
  /**
   * \brief Configure script parameters
   * \param argc is the command line argument count
   * \param argv is the command line arguments
   * \return true on successful configuration
  */
  bool Configure (int argc, char **argv);
  /// Run simulation
  void Run ();
  /**
   * Report results
   * \param os the output stream
   */
  void Report (std::ostream & os);

private:

  // parameters
  /// Number of nodes
  uint32_t size;
  /// Simulation time, seconds
  double totalTime;
  /// Write per-device PCAP traces if true
  bool pcap;
  /// Print routes if true
  bool printRoutes;

  // network
  /// nodes used in the example
  NodeContainer nodes;
  /// devices used in the example
  NetDeviceContainer devices;
  /// interfaces used in the example
  Ipv4InterfaceContainer interfaces;

private:
  /// Create the nodes
  void CreateNodes ();
  /// Create the devices
  void CreateDevices ();
  /// Create the network
  void InstallInternetStack ();
  /// Create the simulation applications
  void InstallApplications ();
};

int main (int argc, char **argv)
{
  AodvExample test;
  if (!test.Configure (argc, argv))
    NS_FATAL_ERROR ("Configuration failed. Aborted.");

  test.Run ();
  test.Report (std::cout);
  return 0;
}

//-----------------------------------------------------------------------------
AodvExample::AodvExample () :
  size (7),
  totalTime (100),
  pcap (true),
  printRoutes (true)
{
}

bool
AodvExample::Configure (int argc, char **argv)
{

  bool verbose = false;

  SeedManager::SetSeed (12345);
  CommandLine cmd;

  cmd.AddValue ("pcap", "Write PCAP traces.", pcap);
  cmd.AddValue ("printRoutes", "Print routing table dumps.", printRoutes);
  cmd.AddValue ("time", "Simulation time, s.", totalTime);
  cmd.AddValue ("time", "Simulation time, s.", totalTime);
  cmd.AddValue ("verbose", "turn on the log", verbose);

  cmd.Parse (argc, argv);

  if (verbose)
    {
      LogComponentEnable ("SimpleAodvTrustManager", LOG_LEVEL_INFO);
    }

  return true;
}

void
AodvExample::Run ()
{
  CreateNodes ();
  CreateDevices ();
  InstallInternetStack ();
  InstallApplications ();

  Simulator::Stop (Seconds (totalTime));

  AnimationInterface anim ("aodv7n.xml");
  anim.EnablePacketMetadata(true);

  Simulator::Run ();
  Simulator::Destroy ();
}

void
AodvExample::Report (std::ostream &)
{ 
}

void
AodvExample::CreateNodes ()
{
  std::cout << "Creating " << (unsigned)size << " nodes.\n";
  nodes.Create (size);
  // Name nodes
  for (uint32_t i = 0; i < size; ++i)
    {
      std::ostringstream os;
      os << "node-" << i;
      Names::Add (os.str (), nodes.Get (i));
    }
  // Create static grid
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject <ListPositionAllocator>();

  // The nodes are placed in a pentagon shape.
  // Nodes can communicate only with their closest neighbor thanks to the fixed modulation type.
  // The fastest route from node 0 to node 3 is through node 1 and 2
  positionAlloc ->Add(Vector(100, 100, 0)); // node0
  positionAlloc ->Add(Vector(174, 46, 0)); // node1
  positionAlloc ->Add(Vector(250, 100, 0)); // node2
  positionAlloc ->Add(Vector(350, 100, 0)); // node3
  positionAlloc ->Add(Vector(321, 188, 0)); // node4
  positionAlloc ->Add(Vector(221, 188, 0)); // node5
  positionAlloc ->Add(Vector(127, 188, 0)); // node6
  mobility.SetPositionAllocator(positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);
}

void
AodvExample::CreateDevices ()
{
  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));
  devices = wifi.Install (wifiPhy, wifiMac, nodes);

  if (pcap)
    {
      wifiPhy.EnablePcapAll (std::string ("aodv"));
    }
}

void
AodvExample::InstallInternetStack ()
{
  NodeContainer normalNodes;
  normalNodes.Add (nodes.Get(0));
  normalNodes.Add (nodes.Get(1));
  normalNodes.Add (nodes.Get(2));
  normalNodes.Add (nodes.Get(3));
  normalNodes.Add (nodes.Get(4));
  normalNodes.Add (nodes.Get(5));

  NodeContainer maliciousNodes;
  maliciousNodes.Add (nodes.Get(6));

  TrustAodvHelper aodv;
  // you can configure AODV attributes here using aodv.Set(name, value)
  InternetStackHelper stack;
  stack.SetRoutingHelper (aodv); // has effect on the next Install ()
  stack.Install (normalNodes);

  MaliciousAodvHelper malAodv;
  stack.SetRoutingHelper (malAodv); // has effect on the next Install ()
  stack.Install (maliciousNodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.0.0.0");
  interfaces = address.Assign (devices);

  if (printRoutes)
    {
      Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("trust-aodv_routing_table_at_55.routes", std::ios::out);
      aodv.PrintRoutingTableAllAt (Seconds (55), routingStream);

      TrustManagerHelper trustManagerHelper;
      Ptr<OutputStreamWrapper> trustRoutingStream = Create<OutputStreamWrapper> ("trust-aodv_trust_table_at_55.routes", std::ios::out);
      trustManagerHelper.PrintTrustTableAllAt (Seconds (55), trustRoutingStream);

      Ptr<OutputStreamWrapper> routingStream3 = Create<OutputStreamWrapper> ("trust-aodv_routing_table_at_75.routes", std::ios::out);
      aodv.PrintRoutingTableAllAt (Seconds (75), routingStream3);

      TrustManagerHelper trustManagerHelper3;
      Ptr<OutputStreamWrapper> trustRoutingStream3 = Create<OutputStreamWrapper> ("trust-aodv_trust_table_at_75.routes", std::ios::out);
      trustManagerHelper3.PrintTrustTableAllAt (Seconds (75), trustRoutingStream3);
    }
}

void
AodvExample::InstallApplications ()
{

  for (uint32_t i = 0; i < size; i++)
    {

      for (uint32_t j = 0; j < size; j++)
        {
          if (i == ( (size - 1) - j))
            {
              continue;
            }
          V4PingHelper ping (interfaces.GetAddress ( (size - 1) - j));
          ping.SetAttribute ("Verbose",
                             BooleanValue (false));

          ApplicationContainer p = ping.Install (nodes.Get (i));
          p.Start (Seconds (0 + (i * 10)));
          p.Stop (Seconds (50) - Seconds (0.001));
        }

    }

  // We want to check the route between 0 and 4.
  // Normally it should pass through node 1 and 2.
  V4PingHelper ping (interfaces.GetAddress (0));
  ping.SetAttribute ("Verbose",
                     BooleanValue (true));

  ApplicationContainer p = ping.Install (nodes.Get (4));
  p.Start (Seconds (60));
  p.Stop (Seconds (70) - Seconds (0.001));
}

