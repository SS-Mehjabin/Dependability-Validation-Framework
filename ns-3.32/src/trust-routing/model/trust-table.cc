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

#include "trust-table.h"
#include "ns3/simulator.h"

namespace ns3 {

TrustTable::TrustTable ()
{
}

bool TrustTable::AddRecord (Address address,
                            double trustValue)
{
  if (trustValue < 0)
    {
      return false;
    }
  TrustEntry newTrustEntry;
  newTrustEntry.SetAddress (address);
  newTrustEntry.SetTrustValue (trustValue);
  newTrustEntry.SetTimestamp (Simulator::Now ());
  std::pair<std::map<Address, TrustEntry>::iterator, bool> result = m_tableRecords.insert (std::make_pair (address,
                                                                                                           newTrustEntry));
  return result.second;
}

bool TrustTable::RemoveRecord (Address address)
{
  if (m_tableRecords.erase (address) != 0)
    {
      return true;
    }
  return false;
}

bool TrustTable::UpdateRecord (Address address,
                               double trustValue)
{
  if (trustValue < 0)
    {
      return false;
    }

  std::map<Address, TrustEntry>::iterator i = m_tableRecords.find (address);
  if (i == m_tableRecords.end ())
    {
      return false;
    }
  TrustEntry newTrustEntry;
  newTrustEntry.SetAddress (address);
  newTrustEntry.SetTrustValue (trustValue);
  newTrustEntry.SetTimestamp (Simulator::Now ());

  i->second = newTrustEntry;
  return true;
}

bool TrustTable::LookupTrustEntry (Address dst,
                                   TrustEntry & tt)
{
  if (m_tableRecords.empty ())
    {
      return false;
    }
  std::map<Address, TrustEntry>::const_iterator i = m_tableRecords.find (dst);
  if (i == m_tableRecords.end ())
    {
      return false;
    }
  tt = i->second;
  return true;
}

bool TrustTable::AddOrUpdateTrustTableEntry (Address dst,
                                             double trustValue)
{
  if (trustValue < 0)
    {
      return false;
    }

  std::map<Address, TrustEntry>::const_iterator i = m_tableRecords.find (dst);

  if (i == m_tableRecords.end ())
    {
      return AddRecord (dst,
                        trustValue);
    }
  else
    {
      return UpdateRecord (dst,
                           trustValue);
    }
}

void TrustTable::Print (Ptr<OutputStreamWrapper> stream) const
{
  *stream->GetStream () << "\nAODV Trust table\n" << "Neighbor\tTrust Value\t\tTimeStamp\n";
  for (std::map<Address, TrustEntry>::const_iterator i = m_tableRecords.begin (); i != m_tableRecords.end (); ++i)
    {
      i->second.Print (stream);
    }
  *stream->GetStream () << "\n";
}


void TrustTable::Purge ()
{
  for (std::map<Address, TrustEntry>::iterator i = m_tableRecords.begin (); i != m_tableRecords.end ();)
    {
      if (Simulator::Now () - i->second.GetTimestamp () > m_lifeTime)
        {
          std::map<Address, TrustEntry>::iterator tmp = i;
          ++i;
          m_tableRecords.erase (tmp);
        }
    }
}


TrustTable::~TrustTable ()
{
}

} // namespace ns3
