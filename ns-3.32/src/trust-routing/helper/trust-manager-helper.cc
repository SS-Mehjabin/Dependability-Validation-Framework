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

#include "trust-manager-helper.h"
#include "ns3/node.h"
#include "ns3/node-list.h"
#include "ns3/simulator.h"

namespace ns3 {

TrustManagerHelper::~TrustManagerHelper ()
{
}

void
TrustManagerHelper::PrintTrustTableAllAt (Time printTime, Ptr<OutputStreamWrapper> stream, Time::Unit unit)
{
  for (uint32_t i = 0; i < NodeList::GetNNodes (); i++)
    {
      Ptr<Node> node = NodeList::GetNode (i);
      Simulator::Schedule (printTime, &TrustManagerHelper::Print, node, stream, unit);
    }
}

void
TrustManagerHelper::PrintTrustTableAllEvery (Time printInterval, Ptr<OutputStreamWrapper> stream, Time::Unit unit)
{
  for (uint32_t i = 0; i < NodeList::GetNNodes (); i++)
    {
      Ptr<Node> node = NodeList::GetNode (i);
      Simulator::Schedule (printInterval, &TrustManagerHelper::PrintEvery, printInterval, node, stream, unit);
    }
}

void
TrustManagerHelper::PrintTrustTableAt (Time printTime, Ptr<Node> node, Ptr<OutputStreamWrapper> stream, Time::Unit unit)
{
  Simulator::Schedule (printTime, &TrustManagerHelper::Print, node, stream, unit);
}

void
TrustManagerHelper::PrintTrustTableEvery (Time printInterval, Ptr<Node> node, Ptr<OutputStreamWrapper> stream, Time::Unit unit)
{
  Simulator::Schedule (printInterval, &TrustManagerHelper::PrintEvery, printInterval, node, stream, unit);
}

void
TrustManagerHelper::Print (Ptr<Node> node, Ptr<OutputStreamWrapper> stream, Time::Unit unit)
{
  for (uint32_t index = 0; index < node->GetNApplications (); index++)
    {
      Ptr<TrustManager> tm = DynamicCast<TrustManager> (node->GetApplication (index));
      if (tm != 0)
        {
          tm->Print (stream, unit);
        }
    }
}

void
TrustManagerHelper::PrintEvery (Time printInterval, Ptr<Node> node, Ptr<OutputStreamWrapper> stream, Time::Unit unit)
{
  for (uint32_t index = 0; index < node->GetNApplications (); index++)
    {
      Ptr<TrustManager> tm = DynamicCast<TrustManager> (node->GetApplication (index));
      if (tm != 0)
        tm->Print (stream, unit);
    }
  Simulator::Schedule (printInterval, &TrustManagerHelper::PrintEvery, printInterval, node, stream, unit);
}

} // namespace ns3
