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

#ifndef SIMPLE_AODV_TRUST_MANAGER_H_
#define SIMPLE_AODV_TRUST_MANAGER_H_

#include "aodv-trust-entry.h"
#include "ns3/trust-manager.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-routing-helper.h"

namespace ns3 {

/**
 * \defgroup trust-aodv
 * \ingroup trust
 *
 * Trust management framework for the AODV protocol.
 */

/**
 * \ingroup trust-aodv
 * \brief This is a demonstration of aodv protocol has been transformed
 * into a trust based version. This has provided a simplest version of
 * trust handling implementation for promiscuous callback function
 * and trust value calculation. This includes user specific implementations
 * that will be used through the trust framework.
 *
 * This trust manager calculates a trust value for each known node in the network
 * based on the ratio of observed RREP to observed RREQ sent by that node.
 * The trust value is then used to select the route, which has the highest trust
 * value when there are multiple possible paths to the same destination.
 */
class SimpleAodvTrustManager : public TrustManager
{
private:
  /**
   * \brief map of AodvTrustEntry objects that contain runtime trust metrics
   * for each directly connected nodes
   */
  std::map<Ipv4Address, AodvTrustEntry> m_trustParameters;

public:
  static TypeId GetTypeId (void);

  SimpleAodvTrustManager ();
  virtual ~SimpleAodvTrustManager ();

  // inherited from Application base class.
  virtual void StartApplication (void);    // Called at time specified by Start
  virtual void StopApplication (void);     // Called at time specified by Stop

  /**
   * \brief Promiscuous callback function which will hooked for nodes.
   * this function will be called upon a packet is being received to the node.
   * \returns bool true if the callback handle the packet successfully
   */
  bool OnReceivePromiscuousCallback (Ptr<NetDevice> device,
                                     Ptr<const Packet> packet,
                                     uint16_t protocol,
                                     const Address &from,
                                     const Address &to,
                                     NetDevice::PacketType packetType);

  /**
   * \brief Calculate the trust value for a given destination node
   * \param [in] address target node IPv4 address to calculate the trust
   * \returns double trust value
   */
  double CalculateTrust (Address ipv4Address);

};


} // namespace ns3
#endif
