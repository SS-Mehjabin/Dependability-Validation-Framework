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

#ifndef SELFISH_AODV_ROUTING_PROTOCOL_H
#define SELFISH_AODV_ROUTING_PROTOCOL_H

#include "ns3/aodv-routing-protocol.h"

namespace ns3 {
namespace selfishaodv {

/**
 * \ingroup aodv
 *
 * \brief Selfish AODV routing protocol
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

  virtual bool RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                           UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                           LocalDeliverCallback lcb, ErrorCallback ecb);

protected:
  virtual void DoInitialize (void);

  ///\name Receive control packets
  //\{
  /// Receive and process control packet
  virtual void RecvAodv (Ptr<Socket> socket);
  /// Receive RREP, but forwards a forget RREP with hop count equal to 1
  void SelfishRecvReply (Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender);

private:
  double m_rrepDropProbability; //!< Probability to drop a RREP to be forwarded
  double m_rreqDropProbability; //!< Probability to drop a RREQ to be forwarded
  double m_dataDropProbability; //!< Probability to drop a data packet to be forwarded
  //\}

  /// @}
};

} //namespace selfishaodv
} //namespace ns3

#endif /* SELFISH_AODV_ROUTING_PROTOCOL_H */
