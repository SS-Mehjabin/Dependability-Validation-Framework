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

#ifndef MALICIOUS_AODV_ROUTING_PROTOCOL_H
#define MALICIOUS_AODV_ROUTING_PROTOCOL_H

#include "ns3/aodv-routing-protocol.h"

namespace ns3 {
namespace maliciousaodv {

/**
 * \ingroup aodv
 *
 * \brief Malicious AODV routing protocol
 */
class RoutingProtocol : public aodv::RoutingProtocol
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  static const uint32_t AODV_PORT;

  /// constructor
  RoutingProtocol ();
  virtual ~RoutingProtocol ();
  virtual void DoDispose ();

protected:
  virtual void DoInitialize (void);

  ///\name Receive control packets
  //\{
  /// Receive and process control packet
  virtual void RecvAodv (Ptr<Socket> socket);
  /// Receive RREP, but forwards a forget RREP with hop count equal to 1
  void MaliciousRecvReply (Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender);

  //\}

  /// @}
};

} //namespace aodv
} //namespace ns3

#endif /* MALICIOUS_AODV_ROUTING_PROTOCOL_H */
