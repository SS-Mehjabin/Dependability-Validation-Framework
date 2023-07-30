.. include:: replace.txt

Trust-based routing protocols framework
---------------------------------------

This module defines the trust-based routing protocols framework which can be used 
to transform an existing routing protocol into a trust based variation.

This module was written by Jude Niroshan from Sri Lanka Institute of Information 
Technology(SLIIT). It has been developed in Google Summer of Code 2018 under the 
mentorship of Tommaso Pecorella from Università di Firenze.

Model Description
*****************

The source code for the new module lives in the directory ``src/trust-routing``.

All the generic classes of trust framework is defined here.

Design
======

Class ``ns3::trust-routing::TrustManager`` serves as the base class for the trust 
framework. This acts as an abstract class which includes a virtual method called 
``CalculateTrust``. This method must be implemented by the extended child class. 
It allows the trust calculation algorithmn decoupled from the trust framework. 

Trust based protocols are operating with trust value. This can be sometimes 
considered as a routing parameter. Therefore, most of the trust based protocols 
are using a trust table which holds trust values for each node in the network. 
Therefore, trust framework has provided a table ``ns3::trust-routing::TrustTable``.
This includes address, trust value and the timestamp of when the value has been 
added to the trust table. 

Class ``ns3::trust-routing::TrustTable`` has a ``std::map`` of 
``ns3::trust-routing::TrustEntry`` objects against the ``ns3::Address``. 
There is only one trust entry instance per given address in the trust 
table. Trust table can hold any node's trust values with no restrictions.

Users of this framework must provide an implementation for 
``ns3::trust-routing::TrustManager``. As TrustManager has extended from 
``ns3::Application``, this can be instantiated and installed to each node of the 
network as an application.

Implementations for ``ns3::trust-routing::TrustManager`` have the flexibility over
what kind of packets should be considered and how the metrics can be manipulated when 
calculating the trust value. 

One method for a trust manager to observe the packet transmissions of the local network 
is to hook the promiscuous callback methods of the relevant NetDevice objects. Trust 
managers may also want to interact with the routing protocol implementations themselves.
This way collected and processed information merged into a single metric called trust 
value. It can be stored in the trust table for future decision making.

Scope and Limitations
=====================

The framework can be used for both IPv4, IPv6. Trust tables are resides for each node in the network.
It can defines a trust value based on directly connected node's communication. It can
print the trust table values to a given output stream and check for the trust entries.

The following features are not yet implemented:

#. Remove trust table entries when a route is removed or invalidated from the 
   trust table
#. Trust value range definition [0, 1] has not been strictly validated. It is the
   responsibility of the framework users to maintain the trust value in this range.
#. Threshold value for trust value should be used to decide whether a node is 
   malicious or healthy

References
==========


Usage
*****

<<need to complete>>
This section is principally concerned with the usage of your model, using
the public API.  Focus first on most common usage patterns, then go
into more advanced topics.

Building New Module
===================


Helpers
=======

<<need to complete>>
What helper API will users typically use?  Describe it here.

Attributes
==========

What classes hold attributes, and what are the key ones worth mentioning?

Output
======

Module do not generate any data by itself. This module is a framework which
accomodates the trust based routing protocol implementations.

Examples
========

Trust framework can not be used alone. It has to be used in a different
module. Examples can be found on such modules. (e.g: trust-aodv)

Troubleshooting
===============

Adding NS_LOG statements will show more runtime data.

Validation
**********

Test coverage has done through trust-routing-test-suite.cc

Future work
===========

# Add a simple boolean method like 'CanTrust()' that returns true if trust
  value is above threshold, false if below. It can be considered to make it
  virtual and allow subclass to override, and then instruct clients of the 
  trust manager to just call CanTrust() instead of CalculateTrust()
