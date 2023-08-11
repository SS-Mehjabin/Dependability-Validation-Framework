Here is the help menu for pyDara.py;

```
(base) tekeoglu@HPENVYdv6:~/Desktop/Dependability-Validation-Framework/pyDARA$ python3 pyDara.py --help
[+HST+] HeartbeatSendThread object is created...
[+HRT+] HeartbeatReceiveThread object is created...
[+FDT+] FailureDetectionThread object is created...
usage: pyDara.py [-h] -n {e1,e2,e3,e4,e5,s1,s2} [-i]

optional arguments:
  -h, --help            show this help message and exit
  -n {e1,e2,e3,e4,e5,s1,s2}, --nodeName {e1,e2,e3,e4,e5,s1,s2}
                        NodeName: should be either e1 through e5 or s1-s2
  -i, --iRobot          If flag is provided, we assume node has a real iRobot.
                        If not provided, iRobot is simulated.

```


pyDARA is run in each node as follows (i.e.; to run on raspberry-pi node e1, connected to a physical iRobot);
```$ python pyDARA.py --nodeName e1 --iRobot```

This will read the config/e1.json file to load two-hop-table for the node.
*--iRobot* option should be provided if the node is connected to an iRobot physically through a USB cable.
If this option is not provided, and if at some point this node needs to move, we will simulate driving iRobot (i.e., calculate amount of time a real iRobot would take to drive there and sleep that long before updating the twoHopTable and notifying new neigbours of arrival/recovery) 


We connect all the Raspberry-Pis to the same access point. 
We use static IPs for each RPI through
```$ vim /etc/dhcpcd.conf```

In this file; modify the wlan0 paragraph for the wireless interface settings, for instance:
```
interface wlan0
static ip_adress=192.168.16.101
static routers=192.168.16.184
static domain_name_servers=192.168.16.184
```

In our setup, the ips of nodes are as follows:

```
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
```

```
e1: 192.168.16.101
e2: 192.168.16.102
e3: 192.168.16.103
e4: 192.168.16.104
e5: 192.168.16.105
s1: 192.168.16.111
s2: 192.168.16.112
```

gateway: 192.168.16.184 (access point/hot-spot).

