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

#ifndef TRUST_MANGER_HELPER_H
#define TRUST_MANGER_HELPER_H

#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/trust-manager.h"

namespace ns3 {

class Node;

/**
 * \ingroup trust
 *
 * \brief a factory to create ns3::TrustManager objects
 *
 * For each new trust manager created as a subclass of
 * ns3::TrustManager, you need to create a subclass of
 * ns3::Ipv4TrustManagerHelper which can be used by
 * to install the proper elements in the node(s).
 */
class TrustManagerHelper
{
public:
  /*
   * Destroy an instance of an TrustManagerHelper
   */
  virtual ~TrustManagerHelper ();

  /**
   * \brief prints the trust tables of all nodes at a particular time.
   * \param printTime the time at which the trust table is supposed to be printed.
   * \param stream The output stream object to use 
   * \param unit The time unit to be used in the report
   *
   * This method calls the PrintTrustTable() method of the
   * TrustManager(s) stored in the node, for all nodes at the
   * specified time; the output format is specific of the trust manager.
   */
  static void PrintTrustTableAllAt (Time printTime, Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S);

  /**
   * \brief prints the trust tables of all nodes at regular intervals specified by user.
   * \param printInterval the time interval for which the trust table is supposed to be printed.
   * \param stream The output stream object to use
   * \param unit The time unit to be used in the report
   *
   * This method calls the PrintTrustTable() method of the
   * TrustManager(s) stored in the node, for all nodes at the
   * specified time interval; the output format is specific of the trust manager.
   */
  static void PrintTrustTableAllEvery (Time printInterval, Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S);

  /**
   * \brief prints the trust tables of a node at a particular time.
   * \param printTime the time at which the trust table is supposed to be printed.
   * \param node The node ptr for which we need the trust table to be printed
   * \param stream The output stream object to use
   * \param unit The time unit to be used in the report
   *
   * This method calls the PrintTrustTable() method of the
   * TrustManager(s) stored in the node, for the selected node
   * specified time; the output format is specific of the trust manager.
   */
  static void PrintTrustTableAt (Time printTime, Ptr<Node> node, Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S);

  /**
   * \brief prints the trust tables of a node at regular intervals specified by user.
   * \param printInterval the time interval for which the trust table is supposed to be printed.
   * \param node The node ptr for which we need the trust table to be printed
   * \param stream The output stream object to use
   * \param unit The time unit to be used in the report
   *
   * This method calls the PrintTrustTable() method of the
   * TrustManager(s) stored in the node, for the selected node
   * specified time interval; the output format is specific of the trust manager.
   */
  static void PrintTrustTableEvery (Time printInterval, Ptr<Node> node, Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S);


  /**
   * \brief Request a specified trust manager &lt;T&gt; installed in the node.
   *
   * \param node Smart pointer to Node object
   * \return a Smart Pointer to the requested manager (zero if the manager can't be found)
   */
  template<class T>
  static Ptr<T> GetTrust (Ptr<Node> node);
  
private:
  /**
   * \brief prints the trust tables of a node.
   * \param node The node ptr for which we need the trust table to be printed
   * \param stream The output stream object to use
   * \param unit The time unit to be used in the report
   *
   * This method calls the PrintTrustTable() method of the
   * TrustManager(s) stored in the node;
   * the output format is specific of the trust manager.
   */
  static void Print (Ptr<Node> node, Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S);

  /**
   * \brief prints the trust tables of a node at regular intervals specified by user.
   * \param printInterval the time interval for which the trust table is supposed to be printed.
   * \param node The node ptr for which we need the trust table to be printed
   * \param stream The output stream object to use
   * \param unit The time unit to be used in the report
   *
   * This method calls the PrintTrustTable() method of the
   * TrustManager(s) stored in the node at the specified interval;
   * the output format is specific of the trust manager.
   */
  static void PrintEvery (Time printInterval, Ptr<Node> node, Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S);
};


/**
 * \brief Request a specified trust protocol &lt;T&gt; from Ipv4TrustProtocol protocol
 *
 * \param node Smart pointer to node object
 * \return a Smart Pointer to the requested protocol (zero if the manager can't be found)
 */
template<class T>
Ptr<T> TrustManagerHelper::GetTrust (Ptr<Node> node)
{
  Ptr<T> ret;

  for (uint32_t index = 0; index < node->GetNApplications (); index++)
    {
      Ptr<T> ret = DynamicCast<T> (node->GetApplication (index));
      if (ret != 0)
        break;
    }
  return ret;
}

} // namespace ns3


#endif /* TRUST_MANGER_HELPER_H */
