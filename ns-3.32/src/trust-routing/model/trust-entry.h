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

#ifndef TRUST_ENTRY_H_
#define TRUST_ENTRY_H_

#include "ns3/address.h"
#include "ns3/nstime.h"
#include "ns3/output-stream-wrapper.h"

namespace ns3 {

/**
 * \ingroup trust
 * \defgroup trust Trust management framework.
 *
 * The Trust Management Framework is built-in support to implement custom
 * trust based protocols in ns-3.
 */

/**
 * \ingroup trust
 * \brief A data structure to hold the trust value and timestamp corresponding
 *        to the address of another node on the network.
 */
class TrustEntry
{
public:
  TrustEntry ();
  virtual ~TrustEntry ();

  /**
   * \breif constructor which accepts all the values for the entry
   */
  TrustEntry (Address dst,
              double trust,
              Time timestamp);

  /**
   * \brief Returns the address of trust table entry
   * \returns the IPv4 address of the node
   */
  Address GetAddress (void) const;

  /**
   * \brief Set the address of the trust table entry
   * \param [in] address address of node
   */
  void SetAddress (Address address);

  /**
   * \brief Get the timestamp of the trust table entry
   * \returns Time object corresponding to the timestamp
   */
  Time GetTimestamp (void) const;

  /**
   * \brief Set the timestamp of the trust table entry
   * \param [in] timestamp Time object corresponding to the timestamp
   */
  void SetTimestamp (Time timestamp);

  /**
   * \brief Get the trust value of the trust table entry
   * \returns trust value
   */
  double GetTrustValue (void) const;

  /**
   * \brief Set the trust value of the trust table entry
   * \param [in] trustValue trust value
   */
  void SetTrustValue (double trustValue);

  /**
   * Print trust entry to output stream
   * \param stream The output stream
   */
  void Print (Ptr<OutputStreamWrapper> stream) const;
private:
  // Address of a node in the network
  Address m_address;
  // trust value for a given node
  double m_trustValue;
  // Timestamp when the trust value has been set
  Time m_timestamp;
};
} // namespace ns3

#endif
