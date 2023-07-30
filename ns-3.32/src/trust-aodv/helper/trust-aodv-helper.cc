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

#include "trust-aodv-helper.h"
#include "ns3/aodv-routing-protocol.h"
#include "ns3/trust-manager.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-list-routing.h"

namespace ns3
{

TrustAodvHelper::TrustAodvHelper () :
  Ipv4RoutingHelper ()
{
  m_agentFactory.SetTypeId ("ns3::aodv::RoutingProtocol");
  m_trustManagerFactory.SetTypeId ("ns3::SimpleAodvTrustManager");
}

TrustAodvHelper*
TrustAodvHelper::Copy (void) const
{
  return new TrustAodvHelper (*this);
}

Ptr<Ipv4RoutingProtocol> 
TrustAodvHelper::Create (Ptr<Node> node) const
{

  Ptr<Application> trustManager = DynamicCast<Application> (m_trustManagerFactory.Create ());
  node->AddApplication (trustManager);
  
  Ptr<aodv::RoutingProtocol> agent = m_agentFactory.Create<aodv::RoutingProtocol> ();
  node->AggregateObject (agent);

  return agent;
}

void 
TrustAodvHelper::Set (std::string name, const AttributeValue &value)
{
  m_agentFactory.Set (name, value);
}

void
TrustAodvHelper::SetManagerType (std::string tid)
{
  m_trustManagerFactory.SetTypeId (tid);
}

void
TrustAodvHelper::SetManagerAttribute (std::string name, const AttributeValue &value)
{
  m_trustManagerFactory.Set (name, value);
}

int64_t
TrustAodvHelper::AssignStreams (NodeContainer c, int64_t stream)
{
  int64_t currentStream = stream;
  Ptr<Node> node;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      node = (*i);
      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
      NS_ASSERT_MSG (ipv4, "Ipv4 not installed on node");
      Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol ();
      NS_ASSERT_MSG (proto, "Ipv4 routing not installed on node");
      Ptr<aodv::RoutingProtocol> aodv = DynamicCast<aodv::RoutingProtocol> (proto);
      if (aodv)
        {
          currentStream += aodv->AssignStreams (currentStream);
          continue;
        }
      // Aodv may also be in a list
      Ptr<Ipv4ListRouting> list = DynamicCast<Ipv4ListRouting> (proto);
      if (list)
        {
          int16_t priority;
          Ptr<Ipv4RoutingProtocol> listProto;
          Ptr<aodv::RoutingProtocol> listAodv;
          for (uint32_t i = 0; i < list->GetNRoutingProtocols (); i++)
            {
              listProto = list->GetRoutingProtocol (i, priority);
              listAodv = DynamicCast<aodv::RoutingProtocol> (listProto);
              if (listAodv)
                {
                  currentStream += listAodv->AssignStreams (currentStream);
                  break;
                }
            }
        }
    }
  return (currentStream - stream);
}

}
