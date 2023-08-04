# Summary
This folder contains the Scapy attack script that is explained in Section-V "Experimental Evaluation"-> D. "ATTACK SCENARIOS" -> (1) Attacking the Trust Scores.
- aodv_packet.pcap contains only one AODV packet, that we extracted from a legitimate AODV traffic, previously.
- aodv_attack.py script uses Scapy to read a legitimate AODV packet from aodv_packet.pcap, modifies it to conform the attack goals, and replays the packet towards the ns-3 victim node.