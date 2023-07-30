<h1>ns-3 Installation and Setup</h1>

ns-3.32 has been used for experimentation. Download the version of ns-3 from : https://www.nsnam.org/releases/ns-3-32/.
Copy the contents of the 'scratch' folder to the ns-3 'scratch' folder.
For running any experimentation regarding trust, copy the contents of 'src' folder to the ns-3 'src' folder. The contents of source folder are modified versions of Trust Based Routing Protocol presented in https://github.com/JudeNiroshan/ns-3-dev-git/tree/gsoc2018_final. 
Use the following commands to build and configure ns-3:

	python3 ./waf configure --enable-tests --enable-sudo
	python3 ./waf build

*****************************************************************************************

<h1>Docker Installation</h1>

Following is a tutorial on how to install docker:

	https://www.simplilearn.com/tutorials/docker-tutorial/how-to-install-docker-on-ubuntu
	
******************************************************************************************

<h1>Creating Docker Overlay Network</h1>

Docker network between host device and physical nodes can be achieved using overlay network. For the overlay network, the host device creates a swarm of which physical nodes become parts. In order to initiate the swarm, execute the following command on the host machine:

	sudo docker swarm init

The following message will appear:

	Swarm initialized: current node (NODE_NAME) is now a manager.

	To add a worker to this swarm, run the following command:

		docker swarm join --token SOME_TOKEN SOME_IP_ADDRESS

	To add a manager to this swarm, run 'docker swarm join-token manager' and follow the instructions.

Now run the following commands for all physical nodes to make them a part of the swarm:

	docker swarm join --token SOME_TOKEN SOME_IP_ADDRESS

The following confirmation message will be displayed:

	This node joined the swarm as a worker

At this point, the host device and the physical nodes are part of a swarm. For them to communicate, a overlay network has to be created between them. Run the following command on host device terminal:

	sudo docker network create -d overlay --attachable NETWORK_NAME

An overlay network has been created named 'NETWORK_NAME'. Two services needs to be created within host device and each physical node using the following command on host machine:

	sudo docker service create --name SERVICE_NAME --network NETWORK_NAME --cap-add=NET_ADMIN --replicas 2 alpine sleep 1d

A service named 'SERVICE_NAME' is created in host and one physical device.

We can create a different overlay network and services for a different physical device in the same way. Now the physical nodes can communicate among themselves after going into the shared service of the respective devices using the command:

	sudo docker exec -it SERVICE_NAME sh
 
Repeat these steps for every ghost node connected to ns-3.
******************************************************************************************

<h1>Creating Network Bridges</h1>

In order for ns-3 to communicate with physical devices, three different bridges need to be considered:

1. Tap Bridge to bring the data packets out of ns-3 to host machine

Following set of commands create the tap bridge for each physical node:

	sudo tunctl -t tap-NAME_OF_TAP_DEVICE
	sudo ifconfig tap-NAME_OF_TAP_DEVICE 0.0.0.0 promisc up

2. Network Bridge to move the packets inside host machine:

Following command creates a linux bridge for each physical node:
	
	sudo brctl addbr br-NAME_OF_TAP_DEVICE

The following command links the tap bridge to the respective linux bridge:
	
	sudo brctl addif br-NAME_OF_TAP_DEVICE tap-NAME_OF_TAP_DEVICE
	sudo ifconfig br-NAME_OF_TAP_DEVICE up

3. Bridge to connect linux host to docker container of each host:

Following sets of commands connect each service of host machine linked to a physical node to the respective linux bridge:

	PID=$(sudo docker inspect --format '{{ .State.Pid }}' NAME_OF_RESPECTIVE_SERVICE_IN_HOST_MACHINE)
	hexchars="0123456789ABCDEF"
	end=$( for i in {1..8} ; do echo -n ${hexchars:$(( $RANDOM % 16 )):1} ; done | sed -e 's/\(..\)/:\1/g' )
	MAC_ADDR="12:34"$end

	sudo mkdir -p /var/run/netns
	sudo ln -s /proc/$PID/ns/net /var/run/netns/$PID
	sudo ip link add side-int-NAME_OF_TAP_DEVICE type veth peer name side-ext-NAME_OF_TAP_DEVICE
	sudo brctl addif br-NAME_OF_TAP_DEVICE side-int-NAME_OF_TAP_DEVICE
	sudo ip link set side-int-NAME_OF_TAP_DEVICE up
	sudo ip link set side-ext-NAME_OF_TAP_DEVICE netns $PID
	sudo ip netns exec $PID ip link set dev side-ext-NAME_OF_TAP_DEVICE name eth3
	sudo ip netns exec $PID ip link set eth3 address $MAC_ADDR
	sudo ip netns exec $PID ip link set eth3 up
	sudo ip netns exec $PID ip addr add IP_ADDRESS_OF_RESPECTIVE_GHOST_NODE_IN_NS3/16 dev eth3
	
Repeat these steps for every physical node linked to ghost node.

******************************************************************************************

<h1>Creating Static Routes</h1>
Static routes have to be set from each of the external node to the simulated nodes inside ns-3. Execute the following command from external node:

	ip route add 10.0.1.0/24 via IP_ADDRESS_OF_NEXT_HOP_NODE
 
Repeat the step for every physical node connected to ns-3.
******************************************************************************************

<h1>Deleting All Bridges and Leaving Swarm After Experimentation Concludes</h1>

After experimentation concludes, it is necessary to delete all the tap and network bridges:

	sudo ifconfig br-NAME_OF_TAP_DEVICE down
	sudo brctl delif br-NAME_OF_TAP_DEVICE tap-NAME_OF_TAP_DEVICE
	sudo brctl delbr br-NAME_OF_TAP_DEVICE
	sudo ifconfig tap-NAME_OF_TAP_DEVICE down
	sudo tunctl -d tap-NAME_OF_TAP_DEVICE
	sudo docker service rm SERVICE_NAME
	sudo docker network rm NETWORK_NAME
	sudo docker swarm leave --force
	
******************************************************************************************




























