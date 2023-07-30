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

#ifndef AODV_TRUST_ENTRY_H_
#define AODV_TRUST_ENTRY_H_

#include "ns3/core-module.h"

namespace ns3 {

/**
 * \ingroup internet
 * \defgroup trust Trust management framework.
 *
 * The Trust Management Framework is built-in support to implement custom
 * trust based protocols in ns-3.
 */

/**
 * \ingroup trust
 * \brief The aodv specific trust entry object which represent rows in trust table.
 */
class AodvTrustEntry
{
private:

  uint16_t m_rreq; //!< number of rreq messages
  uint16_t m_rrep; //!< number of reply messages
  uint16_t m_rerr; //!< number of error messages
  uint16_t m_hello; //!< number of hello messages

public:
  AodvTrustEntry ();
  virtual ~AodvTrustEntry ();

  /**
   * \brief Returns the error count of trust table entry
   * \returns uint16_t of the error count
   */
  uint16_t GetRerrCounter () const;

  /**
   * \brief Set the error count of trust table entry
   * \param [in] err uint16_t of the error count
   */
  void SetRerrCounter (uint16_t err);

  /**
   * \brief Increments the error count of trust table entry
   */
  void IncRerrCounter ();

  /**
   * \brief Returns the hello count of trust table entry
   * \returns uint16_t of the hello count
   */
  uint16_t GetHelloCounter () const;

  /**
   * \brief Set the hello packet count of trust table entry
   * \param [in] hello uint16_t of the hello packet count
   */
  void SetHelloCounter (uint16_t hello);

  /**
   * \brief Increments the hello count of trust table entry
   */
  void IncHelloCounter ();

  /**
   * \brief Returns the reply count of trust table entry
   * \returns uint16_t of the reply count
   */
  uint16_t GetRrepCounter () const;

  /**
   * \brief Set the reply count of trust table entry
   * \param [in] reply uint16_t of the reply count
   */
  void SetRrepCounter (uint16_t rply);

  /**
   * \brief Increments the router reply count of trust table entry
   */
  void IncRrepCounter ();

  /**
   * \brief Returns the rreq count of trust table entry
   * \returns uint16_t of the rreq count
   */
  uint16_t GetRreqCounter () const;

  /**
   * \brief Set the rreq count of trust table entry
   * \param [in] rreq uint16_t of the rreq count
   */
  void SetRreqCounter (uint16_t rreq);

  /**
   * \brief Increments the router request count of trust table entry
   */
  void IncRreqCounter ();

};
  std::ostream &operator<< (std::ostream &os, AodvTrustEntry const &m);

} // namespace ns3
#endif
