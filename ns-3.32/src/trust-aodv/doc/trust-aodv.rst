.. include:: replace.txt
.. highlight:: cpp

Trust AODV Module Documentation
----------------------------

This model was written as an example demonstration for transforming the 
traditional AODV protocol into a trust-based AODV protocol which uses trust
to delegate packets within the network. It used trust framework(trust-routing) 
to access the ns-3 core modules.

This module was written by Jude Niroshan from Sri Lanka Institute of Information 
Technology(SLIIT). It has been developed in Google Summer of Code 2018 under the 
mentorship of Tommaso Pecorella from Università di Firenze.


Model Description
*****************

The source code for the new module lives in the directory ``src/trust-aodv``.

Module contains overriden protocol called as ``ns3::trustaodv::RoutingProcotol``
which extended from ``ns3::aodv::RoutingProtocol``. Protocol has changed in a way 
to utilize the trust values when forwarding packets.

Design
======

Module has the usual ns-3 module architecture except that it does not include 
test cases at the initial release. We hope to write the unit test which covers 
major protocol overriden scenarios in future. Some of the implementations that 
includes in this module is primivitive and the implementations are not focused 
on a particular publication. The main purpose of this module is to provide the 
knowledge on when implementing an trust based routing protocol on ns-3, how we 
can utilize the trust framework. 

trust-aodv has direct dependencies on trust-routing and aodv modules. Class 
``ns3::trustaodv::SimpleAodvTrustManager`` is the implementation to
``ns3::trustrouting::TrustManager`` which defined in trust framework. This class 
includes the trust calculation logic and how the promiscuous callback function 
should react upon receiving intended packets to network nodes. It also holds 
trust parameter table which consists of ``ns3::trustaodv::AodvTrustEntry`` 
objects. trust parameter table is a different table to trust table. It contains 
parameters that can be considered when calculating the trust value. As we can 
see almost all the trust based routing protocols have different mechanisms to 
calculate the trust value. Therefore, this table is to manage such attributes 
for the trust value. Even though we have number of rreq, reply, error and hello 
counters, in our demonstration we have only used ``m_rreq`` and ``m_rrep``. 

There are 2 different types of aodv-protocol overriden files are available 
under the model folder. ``ns3::maliciousaodv::RoutingProtocol`` and 
``ns3::selfishaodv::RoutingProtocol``. These two protocol classes are used 
to simulate a malicious node behaviour in our provided example. Malicious aodv 
implementation is able to modify the number of hops which sends out in RREP 
message. It is done by simply reducing 1 from the available hops counts in a 
RREP packet. With regard to selfish node behavious, it will randomly drop 
RREP packets without forwarding to the next hop.

For each of the above mentioned routing protocol overriden versions we have 
separate helper classes inside the helper folder. These helpers can be used in 
any simulation examples. Those have been used in trust-aodv-example simulation 
to install the intended overriden routing protocols. 

Scope and Limitations
=====================

This is a demonstration/pilot module which can be referred to understand 
how to utilize the trust framework when implementing a trust based routing
protocol in ns-3. It defines how to manipulate an existing routing protocol 
to work based on a trust measurement. 

The following implementations are subject to debate:

#. Algorithm used to calculate the trust value
#. Packets that are being snoofing through promiscuous callback function
#. Threahold value that used to define a malicious node

References
==========


Usage
*****

This module is mainly used as a reference guide on how to implement your 
own trust based routing protocol in ns-3 without modifying the existing 
ns-3 modules.

Helpers
=======

All the existing helpers are extended implementations for 
``ns3::Ipv4RoutingHelper``. These helpers are simply act as delegators
to their coresponding routing protocol class.

Attributes
==========

What classes hold attributes, and what are the key ones worth mentioning?

``m_trustParameters`` is the trust parameter table that holds all the 
required attribute values to calculate the trust value. It has been 
represented as a table with ``ns3::Ipv4Address`` as the unique identifier 
for the table. It is available in ``ns3::SimpleAodvTrustManager`` class.

``m_rrepDropProbability``, ``m_rreqDropProbability``, ``m_dataDropProbability``
are respectively used inside the ``ns3::selfishaodv::RoutingProtocol`` class 
to decide the probability ratios to drop RREQ and RREP packets in a selfish 
network node.

Output
======

Depending on your simulation setup, you can observe the pcap files 
to understand how the trust based aodv forward packets. According to 
our provided example, you can observe the pcap files for node 0 at 
55th and 75th seconds. Routing table and trust table values has been 
changed over the time while reducing the trust value for the malicious 
node.


Examples
========

There is only one example available right now for this trust based 
aodv variation module. It is a 7 node wireless network setup with 
one malicious node which interrupts the communication by manipulating 
hop counts in the network route reply message. Simulation has 100 seconds 
of duration time and first 50 seconds are used to send ping message to all 
available network nodes. At 60th - 70th node a new ping message is being 
sent and new route discovery process will take place. In this scenario 
the trust values recalculation happens and the results are available 
in pcap files and trust table output files.