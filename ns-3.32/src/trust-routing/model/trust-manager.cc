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

#include "trust-manager.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(TrustManager);

TrustManager::TrustManager ()
{
}

TrustManager::~TrustManager ()
{
}

TypeId TrustManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TrustManager")
    .SetParent<Application> ()
    .SetGroupName ("Trust")
  ;
  return tid;
}

void TrustManager::Print (Ptr<OutputStreamWrapper> stream, Time::Unit unit)
{
  m_trustTable.Print (stream);
}

} // namespace ns3
