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

#include "malicious-aodv-routing-protocol.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MaliciousAodvRoutingProtocol");

namespace maliciousaodv {
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
  static TypeId tid = TypeId ("ns3::maliciousaodv::RoutingProtocol")
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
        MaliciousRecvReply (packet, receiver, sender);
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
RoutingProtocol::MaliciousRecvReply (Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender)
{
  NS_LOG_FUNCTION (this << " src " << sender);
  aodv::RrepHeader rrepHeader;
  p->RemoveHeader (rrepHeader);
  Ipv4Address dst = rrepHeader.GetDst ();
  NS_LOG_LOGIC ("RREP destination " << dst << " RREP origin " << rrepHeader.GetOrigin ());

  uint8_t hop = rrepHeader.GetHopCount () + 1;
  rrepHeader.SetHopCount (hop);

  // If RREP is Hello message
  if (dst == rrepHeader.GetOrigin ())
    {
      ProcessHello (rrepHeader, receiver);
      return;
    }

  /*
   * If the route table entry to the destination is created or updated, then the following actions occur:
   * -  the route is marked as active,
   * -  the destination sequence number is marked as valid,
   * -  the next hop in the route entry is assigned to be the node from which the RREP is received,
   *    which is indicated by the source IP address field in the IP header,
   * -  the hop count is set to the value of the hop count from RREP message + 1
   * -  the expiry time is set to the current time plus the value of the Lifetime in the RREP message,
   * -  and the destination sequence number is the Destination Sequence Number in the RREP message.
   */
  Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (receiver));
  aodv::RoutingTableEntry newEntry (/*device=*/ dev, /*dst=*/ dst, /*validSeqNo=*/ true, /*seqno=*/ rrepHeader.GetDstSeqno (),
                                    /*iface=*/ m_ipv4->GetAddress (m_ipv4->GetInterfaceForAddress (receiver), 0),/*hop=*/ hop,
                                    /*nextHop=*/ sender, /*lifeTime=*/ rrepHeader.GetLifeTime ());
  aodv::RoutingTableEntry toDst;
  if (m_routingTable.LookupRoute (dst, toDst))
    {
      /*
       * The existing entry is updated only in the following circumstances:
       * (i) the sequence number in the routing table is marked as invalid in route table entry.
       */
      if (!toDst.GetValidSeqNo ())
        {
          m_routingTable.Update (newEntry);
        }
      // (ii)the Destination Sequence Number in the RREP is greater than the node's copy of the destination sequence number and the known value is valid,
      else if ((int32_t (rrepHeader.GetDstSeqno ()) - int32_t (toDst.GetSeqNo ())) > 0)
        {
          m_routingTable.Update (newEntry);
        }
      else
        {
          // (iii) the sequence numbers are the same, but the route is marked as inactive.
          if ((rrepHeader.GetDstSeqno () == toDst.GetSeqNo ()) && (toDst.GetFlag () != aodv::VALID))
            {
              m_routingTable.Update (newEntry);
            }
          // (iv)  the sequence numbers are the same, and the New Hop Count is smaller than the hop count in route table entry.
          else if ((rrepHeader.GetDstSeqno () == toDst.GetSeqNo ()) && (hop < toDst.GetHop ()))
            {
              m_routingTable.Update (newEntry);
            }
        }
    }
  else
    {
      // The forward route for this destination is created if it does not already exist.
      NS_LOG_LOGIC ("add new route");
      m_routingTable.AddRoute (newEntry);
    }
  // Acknowledge receipt of the RREP by sending a RREP-ACK message back
  if (rrepHeader.GetAckRequired ())
    {
      SendReplyAck (sender);
      rrepHeader.SetAckRequired (false);
    }
  NS_LOG_LOGIC ("receiver " << receiver << " origin " << rrepHeader.GetOrigin ());
  if (IsMyOwnAddress (rrepHeader.GetOrigin ()))
    {
      if (toDst.GetFlag () == aodv::IN_SEARCH)
        {
          m_routingTable.Update (newEntry);
          m_addressReqTimer[dst].Remove ();
          m_addressReqTimer.erase (dst);
        }
      m_routingTable.LookupRoute (dst, toDst);
      SendPacketFromQueue (dst, toDst.GetRoute ());
      return;
    }

  aodv::RoutingTableEntry toOrigin;
  if (!m_routingTable.LookupRoute (rrepHeader.GetOrigin (), toOrigin) || toOrigin.GetFlag () == aodv::IN_SEARCH)
    {
      return; // Impossible! drop.
    }
  toOrigin.SetLifeTime (std::max (m_activeRouteTimeout, toOrigin.GetLifeTime ()));
  m_routingTable.Update (toOrigin);

  // Update information about precursors
  if (m_routingTable.LookupValidRoute (rrepHeader.GetDst (), toDst))
    {
      toDst.InsertPrecursor (toOrigin.GetNextHop ());
      m_routingTable.Update (toDst);

      aodv::RoutingTableEntry toNextHopToDst;
      m_routingTable.LookupRoute (toDst.GetNextHop (), toNextHopToDst);
      toNextHopToDst.InsertPrecursor (toOrigin.GetNextHop ());
      m_routingTable.Update (toNextHopToDst);

      toOrigin.InsertPrecursor (toDst.GetNextHop ());
      m_routingTable.Update (toOrigin);

      aodv::RoutingTableEntry toNextHopToOrigin;
      m_routingTable.LookupRoute (toOrigin.GetNextHop (), toNextHopToOrigin);
      toNextHopToOrigin.InsertPrecursor (toDst.GetNextHop ());
      m_routingTable.Update (toNextHopToOrigin);
    }
  SocketIpTtlTag tag;
  p->RemovePacketTag (tag);
  if (tag.GetTtl () < 2)
    {
      NS_LOG_DEBUG ("TTL exceeded. Drop RREP destination " << dst << " origin " << rrepHeader.GetOrigin ());
      return;
    }

  Ptr<Packet> packet = Create<Packet> ();
  SocketIpTtlTag ttl;
  ttl.SetTtl (tag.GetTtl () - 1);
  packet->AddPacketTag (ttl);

  // here we forge the hop count
  rrepHeader.SetHopCount (1);

  packet->AddHeader (rrepHeader);
  aodv::TypeHeader tHeader (aodv::AODVTYPE_RREP);
  packet->AddHeader (tHeader);
  Ptr<Socket> socket = FindSocketWithInterfaceAddress (toOrigin.GetInterface ());
  NS_ASSERT (socket);
  socket->SendTo (packet, 0, InetSocketAddress (toOrigin.GetNextHop (), AODV_PORT));
}

} //namespace maliciousaodv
} //namespace ns3
