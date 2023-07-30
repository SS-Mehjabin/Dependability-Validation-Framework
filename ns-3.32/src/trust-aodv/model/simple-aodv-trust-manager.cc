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

#include "simple-aodv-trust-manager.h"
#include "aodv-trust-entry.h"
#include "ns3/aodv-packet.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/aodv-routing-protocol.h"
#include "ns3/mac48-address.h"
#include "ns3/log.h"
#include "ns3/loopback-net-device.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SimpleAodvTrustManager");

NS_OBJECT_ENSURE_REGISTERED(SimpleAodvTrustManager);

TypeId SimpleAodvTrustManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SimpleAodvTrustManager")
    .SetParent<TrustManager> ()
    .SetGroupName ("Aodv")
    .AddConstructor<SimpleAodvTrustManager> ()
  ;
  return tid;
}

SimpleAodvTrustManager::SimpleAodvTrustManager ()
{
}

SimpleAodvTrustManager::~SimpleAodvTrustManager ()
{
}

bool SimpleAodvTrustManager::OnReceivePromiscuousCallback (Ptr<NetDevice> device,
                                                           Ptr<const Packet> packet,
                                                           uint16_t protocol,
                                                           const Address &from,
                                                           const Address &to,
                                                           NetDevice::PacketType packetType)
{
  Ipv4Header ipv4Header;

  Ptr<Packet> copyPacket;
  copyPacket = packet->Copy();

  if (protocol != Ipv4L3Protocol::PROT_NUMBER)
    {
      return false;
    }
  copyPacket->RemoveHeader (ipv4Header);
  Ipv4Address ipv4Address = ipv4Header.GetSource ();

  std::map<Ipv4Address, AodvTrustEntry>::iterator i = m_trustParameters.find (ipv4Address);
  AodvTrustEntry aodvTrustEntry;

  if (i != m_trustParameters.end ())
    {
      aodvTrustEntry = i->second;
    }
  UdpHeader udpHeader;
  copyPacket->RemoveHeader (udpHeader);

  if (udpHeader.GetDestinationPort () != aodv::RoutingProtocol::AODV_PORT)
    {
      return false;
    }

  aodv::TypeHeader tHeader;
  copyPacket->RemoveHeader (tHeader);
  switch (tHeader.Get ())
    {
    case aodv::AODVTYPE_RREQ:
      {
        NS_LOG_INFO("RREQ captured in Promiscuous callback function");
        aodvTrustEntry.IncRreqCounter ();
        break;
      }
    case aodv::AODVTYPE_RREP:
      {
        if (Mac48Address::IsMatchingType (to))
          {
            Mac48Address to48Addr = Mac48Address::ConvertFrom (to);
            if (to48Addr.IsBroadcast ())
              {
                // NS_LOG_INFO("RREP skipped because has been broadcasted");
                break;
              }
          }
        NS_LOG_INFO("RREP captured in Promiscuous callback function");
        aodvTrustEntry.IncRrepCounter ();
        break;
      }
    case aodv::AODVTYPE_RERR:
      {
        NS_LOG_INFO("RERR captured in Promiscuous callback function");
        aodvTrustEntry.IncRerrCounter ();
        break;
      }
    case aodv::AODVTYPE_RREP_ACK:
      {
        NS_LOG_INFO("RREP_ACK captured in Promiscuous callback function");
        break;
      }
    }

  m_trustParameters[ipv4Address] = aodvTrustEntry;
  double calculatedTrust = this->CalculateTrust (ipv4Address);

  m_trustTable.AddOrUpdateTrustTableEntry (ipv4Address,
                                           calculatedTrust);

  return true;
}

double SimpleAodvTrustManager::CalculateTrust (Address address)
{
  Ipv4Address ipv4Address = Ipv4Address::ConvertFrom (address);
  std::map<Ipv4Address, AodvTrustEntry>::iterator i = m_trustParameters.find (ipv4Address);

  if (i == m_trustParameters.end ())
    {
      return 0; // <! No existing data to calculate trust
    }

  AodvTrustEntry m_aodvTrustEntry = i->second;

  double trustDouble;
  if (m_aodvTrustEntry.GetRreqCounter ())
    trustDouble = static_cast<double> (m_aodvTrustEntry.GetRrepCounter ()) / (static_cast<double> (m_aodvTrustEntry.GetRreqCounter ())+static_cast<double> (m_aodvTrustEntry.GetRrepCounter ()));
  else
    trustDouble = 0.5; // we assume that an unknown node is neither trusted or untrusted.

  return trustDouble;
}

// Application Methods
void SimpleAodvTrustManager::StartApplication (void) // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);

  // \todo revise this mechanism
  // The PromiscReceiveCallback idea is mart, but it's not foolproof.
  // If the user installs a second TrustManager or anything that uses PromiscReceiveCallback,
  // then this app will stop working.
  //
  // A more reliable way is to use a Ipv4 / Ipv6 Raw socket.

  for (uint32_t index=0; index<m_node->GetNDevices (); index++)
    {
      Ptr<NetDevice> device = m_node->GetDevice (index);
      if (!DynamicCast<LoopbackNetDevice> (device))
        {
          device->SetPromiscReceiveCallback (ns3::MakeCallback (&SimpleAodvTrustManager::OnReceivePromiscuousCallback, this));
          m_trustTable.Purge (); // TODO make to run repititively with a given time interval
        }
    }
}

void SimpleAodvTrustManager::StopApplication (void) // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);

  for (uint32_t index=0; index<m_node->GetNDevices (); index++)
    {
      Ptr<NetDevice> device = m_node->GetDevice (index);
      if (!DynamicCast<LoopbackNetDevice> (device))
        {
          NetDevice::PromiscReceiveCallback cb = ns3::MakeNullCallback< bool,
                                                                        Ptr<NetDevice>,
                                                                        Ptr<const Packet>,
                                                                        uint16_t,
                                                                        const Address &,
                                                                        const Address &,
                                                                        enum NetDevice::PacketType > ();
          device->SetPromiscReceiveCallback (cb);
          m_trustTable.Purge ();
        }
    }
}

}
