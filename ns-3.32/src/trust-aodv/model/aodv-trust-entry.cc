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

#include "aodv-trust-entry.h"

namespace ns3
{

AodvTrustEntry::AodvTrustEntry ()
{
  m_rreq = 0;
  m_rrep = 0;
  m_rerr = 0;
  m_hello = 0;
}

AodvTrustEntry::~AodvTrustEntry ()
{
}

uint16_t AodvTrustEntry::GetRerrCounter () const
{
  return m_rerr;
}

void AodvTrustEntry::SetRerrCounter (uint16_t err)
{
  m_rerr = err;
}

void AodvTrustEntry::IncRerrCounter ()
{
  m_rerr++;
}

uint16_t AodvTrustEntry::GetHelloCounter () const
{
  return m_hello;
}

void AodvTrustEntry::SetHelloCounter (uint16_t hello)
{
  m_hello = hello;
}

void AodvTrustEntry::IncHelloCounter ()
{
  m_hello++;
}

uint16_t AodvTrustEntry::GetRrepCounter () const
{
  return m_rrep;
}

void AodvTrustEntry::SetRrepCounter (uint16_t rply)
{
  m_rrep = rply;
}

void AodvTrustEntry::IncRrepCounter ()
{
  m_rrep++;
}

uint16_t AodvTrustEntry::GetRreqCounter () const
{
  return m_rreq;
}

void AodvTrustEntry::SetRreqCounter (uint16_t rreq)
{
  m_rreq = rreq;
}

void AodvTrustEntry::IncRreqCounter ()
{
  m_rreq++;
}

std::ostream &operator<< (std::ostream &os, AodvTrustEntry const &m)
{
  os << "rreq=" << m.GetRreqCounter () << " | " << "rrep=" << m.GetRrepCounter ();
  return os;
}

}
