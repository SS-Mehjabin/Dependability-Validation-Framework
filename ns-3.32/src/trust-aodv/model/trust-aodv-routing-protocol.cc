/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 Universita' di Firenze, Italy
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
 * Author: Tommaso Pecorella <tommaso.pecorella@unifi.it>
 */

#include "trust-aodv-routing-protocol.h"
#include "simple-aodv-trust-manager.h"
#include "ns3/trust-manager.h"
#include "ns3/log.h"
#include "ns3/double.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TrustAodvRoutingProtocol");

namespace trustaodv {
NS_OBJECT_ENSURE_REGISTERED (RoutingProtocol);

/// UDP Port for AODV control traffic
const uint32_t RoutingProtocol::AODV_PORT = 654;


//-----------------------------------------------------------------------------
RoutingProtocol::RoutingProtocol ()
{
}

TypeId
RoutingProtocol::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::trustaodv::RoutingProtocol")
    .SetParent<aodv::RoutingProtocol> ()
    .SetGroupName ("Aodv")
    .AddConstructor<RoutingProtocol> ()
  ;
  return tid;
}

RoutingProtocol::~RoutingProtocol ()
{
}

void
RoutingProtocol::DoDispose ()
{
  aodv::RoutingProtocol::DoDispose ();
}

void
RoutingProtocol::RecvAodv (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Address sourceAddress;
  Ptr<Packet> packet = socket->RecvFrom (sourceAddress);
  InetSocketAddress inetSourceAddr = InetSocketAddress::ConvertFrom (sourceAddress);
  Ipv4Address sender = inetSourceAddr.GetIpv4 ();
  Ipv4Address receiver;

  // std::cout << "Malicious AODV routing received an AODV signalling pkt" << std::endl;

  if (m_socketAddresses.find (socket) != m_socketAddresses.end ())
    {
      receiver = m_socketAddresses[socket].GetLocal ();
    }
  else if (m_socketSubnetBroadcastAddresses.find (socket) != m_socketSubnetBroadcastAddresses.end ())
    {
      receiver = m_socketSubnetBroadcastAddresses[socket].GetLocal ();
    }
  else
    {
      NS_ASSERT_MSG (false, "Received a packet from an unknown socket");
    }
  NS_LOG_DEBUG ("AODV node " << this << " received a AODV packet from " << sender << " to " << receiver);

  UpdateRouteToNeighbor (sender, receiver);
  aodv::TypeHeader tHeader (aodv::AODVTYPE_RREQ);
  packet->RemoveHeader (tHeader);
  if (!tHeader.IsValid ())
    {
      NS_LOG_DEBUG ("AODV message " << packet->GetUid () << " with unknown type received: " << tHeader.Get () << ". Drop");
      return; // drop
    }
  switch (tHeader.Get ())
    {
    case aodv::AODVTYPE_RREQ:
      {
        RecvRequest (packet, receiver, sender);
        break;
      }
    case aodv::AODVTYPE_RREP:
      {
        TrustRecvReply (packet, receiver, sender);
        break;
      }
    case aodv::AODVTYPE_RERR:
      {
        RecvError (packet, sender);
        break;
      }
    case aodv::AODVTYPE_RREP_ACK:
      {
        RecvReplyAck (sender);
        break;
      }
    }
}

void
RoutingProtocol::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  aodv::RoutingProtocol::DoInitialize ();
}

void
RoutingProtocol::TrustRecvReply (Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender)
{
  NS_LOG_FUNCTION (this << " src " << sender);

  aodv::RrepHeader rrep;
  p->PeekHeader (rrep);
  Ipv4Address dst = rrep.GetDst ();
  Ipv4Address actualNextHop;

  aodv::RoutingTableEntry rt;
  if (m_routingTable.LookupValidRoute (dst, rt))
    {
      actualNextHop = rt.GetNextHop ();
    }
  else
    {
      // We don't know any next hop for that destination yet, we need to accept this one.
      RecvReply (p, receiver, sender);
      return;
    }

  Ptr<Node> node = m_ipv4->GetObject<Node> ();
  double newNodeAvgTrustValue = 0.5;
  double newNodeTrustValueSum = 0;
  uint32_t newNodeTrustManagersNum = 0;
  double oldNodeAvgTrustValue = 0.5;
  double oldNodeTrustValueSum = 0;
  uint32_t oldNodeTrustManagersNum = 0;

  for (uint32_t index=0; index<node->GetNApplications (); index++)
    {
      Ptr<TrustManager> manager = DynamicCast<TrustManager> (node->GetApplication (index));
      if (manager)
        {
          TrustEntry tt;
          bool found;
          found = manager->m_trustTable.LookupTrustEntry (sender, tt);
          if (found)
            {
              newNodeTrustManagersNum ++;
              newNodeTrustValueSum += tt.GetTrustValue ();
            }
          found = manager->m_trustTable.LookupTrustEntry (actualNextHop, tt);
          if (found)
            {
              oldNodeTrustManagersNum ++;
              oldNodeTrustValueSum += tt.GetTrustValue ();
            }
        }
    }
  if (newNodeTrustValueSum)
    {
      newNodeAvgTrustValue = newNodeTrustValueSum / newNodeTrustManagersNum;
    }
  if (oldNodeTrustValueSum)
    {
      oldNodeAvgTrustValue = oldNodeTrustValueSum / oldNodeTrustManagersNum;
    }

  // We have 4 cases:
  // 1) both next hops are above threshold.
  // 2) The old one is below, and the new one is above
  // 3) The old one is above, and the new one is below
  // 4) both are below.
  // The result is:
  // 1 - Let the hop number decide
  // 2 - discard the old one
  // 3 - discard the new one
  // 4 - choose the one with highest trust

  // Case 1
  if ((oldNodeAvgTrustValue >= 0.5) && (newNodeAvgTrustValue >= 0.5))
    {
      RecvReply (p, receiver, sender);
    }
  // Case 2
  else if ((oldNodeAvgTrustValue < 0.5) && (newNodeAvgTrustValue >= 0.5))
    {
      m_routingTable.DeleteRoute (dst);
      RecvReply (p, receiver, sender);
    }
  // Case 4
  else if ((oldNodeAvgTrustValue < 0.5) && (newNodeAvgTrustValue < 0.5))
    {
      if (newNodeAvgTrustValue > oldNodeAvgTrustValue)
        {
          m_routingTable.DeleteRoute (dst);
          RecvReply (p, receiver, sender);
        }
    }
  return;
}

} //namespace selfishaodv
} //namespace ns3
