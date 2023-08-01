pyDARA is run in each node as follows (to run on node e1);

$ python pyDARA.py --nodeName config/e3.json


We connect all the Raspberry-Pis to the same access point.
Set static IPs for each RPI through
$ vim /etc/dhcpcd.conf

In this file modify the wlan0 for wireless interface, for instance:

interface wlan0
static ip_adress=192.168.61.101
static routers=192.168.61.1
static domain_name_servers=192.168.61.169


In our setup, the ips of nodes are as follows:


         e2 <---> e3
         ^
         |
         |
s2 <---> e1 <---> s1
         |
         |
    _____|________
    |            |
    |            |
    e4           e5

e1: 192.168.61.101
e2: 192.168.61.102
e3: 192.168.61.103
e4: 192.168.61.104
e5: 192.168.61.105
s1: 192.168.61.111
s2: 192.168.61.112

gw: 192.168.61.169 (access point/hot-spot)