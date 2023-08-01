import json, os, socket, sys
import math
import random
import threading
import socket
import time
import datetime
import argparse
from subprocess import check_output as runoutput

from flask import Flask, request, jsonify

app = Flask(__name__)

# DARA-1C variables
global node_name
global coord_x
global coord_y
global twoHopTable
global lst_oneHopIPs
global udp_port

######
# DARA-1C every node needs to run a thread that Broadcasts a message as a heartbeat to it's 1-link neighbours and 
# listens in another thread to incoming heartbeat from 1-link neighbours.
#####
class HeartbeatSendThread(threading.Thread):
    print(f"[+] HeartbeatSendThread object is created...")
    def run(self):
        print("[+] Inside SendThread run...")
        client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        #client.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1) # TODO: Do not Broadcast, send to one-link neighbours.
        client.settimeout(0.2)
        #msg = bytes(f'This is -{os.environ.get("HOSTNAME")}-', 'utf-8')
        msg = bytes(f'This is -{node_name}-', 'utf-8')

        while True:
            #client.sendto(msg, ('<broadcast>', 37020))
            for ip in lst_oneHopIPs:
                client.sendto(msg, (ip, udp_port))
                print(f'[+] H-B-Msg sent to {ip}:{udp_port}...(at {datetime.datetime.now()})')

            time.sleep(5)

class HeartbeatReceiveThread(threading.Thread):
    print(f"[+] HeartbeatReceiveThread object is created...")
    def run(self):
        print("[+] Inside ReceiveThread run...")
        client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        #client.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1) # Enable Broadcasting Mode
        client.bind(("", udp_port))
        while True:
            print("[+] RcvThread: waiting for incoming MSG....")
            data, addr = client.recvfrom(1024)
            print(f"[+] Received broadcast MSG: {data} from {addr}. (at {datetime.datetime.now()})")


######
# Load the 2-hop Table for the node from the file (under config/ folder)
#####
def loadTwoHopTable(nodeName):
    global twoHopTable
    global node_name
    global lst_oneHopIPs
    global udp_port
    script_dir = os.path.dirname(__file__)
    with open( os.path.join(script_dir, f'configs/{nodeName}.json') ) as jsf:
        twoHopTable = json.load(jsf)
    
    if twoHopTable["name"] in nodeName:
        node_name = twoHopTable["name"]
        lst_oneHopIPs = getOneHopNeighbourIPs()
        udp_port = 37020
        return True
    else:
        return False

##############################################################################
# Helper functions for DARA-1C
#####
def getFailedNeighbour():
    failedNeighbours=[]
    return failedNeighbours


def getDistToNode(neighbourName):   # could be a one-hop or two-hop neighbour
    found=False
    my_x=float(twoHopTable['coords'].split(",")[0])
    my_y=float(twoHopTable['coords'].split(",")[1])
    x=-1
    y=-1

    for oneHopNeigh in twoHopTable["links"]:
        if neighbourName in oneHopNeigh["name"]:
            print(f"[+] Found neighbour ({neighbourName}) in one-link. Coords: {oneHopNeigh['coords']}")
            x=float(oneHopNeigh['coords'].split(",")[0])
            y=float(oneHopNeigh['coords'].split(",")[1])
            found=True
            break

    if not found:       # if the node we are looking for not found in one-hop neighbours, look for 2-hop neighbours
        for oneHopNeigh in twoHopTable["links"]:
            for secondHopNeigh in oneHopNeigh["links"]:
                if neighbourName in secondHopNeigh["name"]:
                    print(f"[+] Found neighbour ({neighbourName}) in two-link. Coords: {secondHopNeigh['coords']}")
                    x=float(secondHopNeigh['coords'].split(",")[0])
                    y=float(secondHopNeigh['coords'].split(",")[1])
                    found=True
                    break

    if x == -1 and y == -1 and not found:
        print(f"[!] Couldn't find {neighbourName} in the twoHopTable:{twoHopTable}")
        return -1
    else:
        return math.sqrt( (my_x - x)**2 + (my_y -y)**2 )
    

def getOneHopNeighbourIPs():
    lst=[]
    for oneHopNeigh in twoHopTable["links"]:
        lst.append(oneHopNeigh["ip"])
    return lst

def getOneHopNeighbourNames():
    lst=[]
    for oneHopNeigh in twoHopTable["links"]:
        lst.append(oneHopNeigh["name"])
    return lst


def getOneHopNeighbourIPandNames():
    lst=[]
    for oneHopNeigh in twoHopTable["links"]:
        lst.append( ( oneHopNeigh["name"], oneHopNeigh["ip"] ) )
    return lst


##############################################################################
# DARA-1C functions
#####
""" TODO:   If a 1-hop neighbour doesn't send heartbeat in a certain amount of time, we 
            assume it needs to be replaced by closest and least degree 1-hop neighbour.
            This function needs to figure out the distance from the failing (1-hop neighbour)
            node to all of it's neighbours. Taking into accout the degrees of nodes."""
def findBestCandidate():
    twoHopTable
    return

""" TODO:   This will be calling the iRobot script to move the iRobot into that coordintes"""
def moveToLocation(coord):
    return

""" TODO:   If we move, or a neighbour moves, we need to update the twoHopTable to reflect the new topology"""
def updateTwoHopTable():
    twoHopTable
    return

""" TODO:   If our parent moves, we need to run this function."""
def childMoveOptimizer():
    return


################################################################################
@app.route('/')
def hello_world():
    return f'Hello, Docker Container Name: {os.environ["HOSTNAME"]}, ip:{get_ip_address()}\n'

@app.route('/ip')
def get_ip_address():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(("8.8.8.8", 80))
    return s.getsockname()[0]

@app.route('/verifier_ip')
def get_verifier_ip_address():
    return os.environ["VERIFIER"]

@app.route('/prover_ip')
def get_prover_ip_address():
    return os.environ["PROVER"]
################################################################################

if __name__ == "__main__":
    """
    if os.environ["HOSTNAME"] is None:
        print(f"[+] Couldn't find the HOSTNAME for this container. Make sure to pass the ENV with -e")
    """

    parser = argparse.ArgumentParser()
    parser.add_argument("-n", "--nodeName", choices=['e1', 'e2', 'e3', 'e4', 'e5', 'n1', 'n2'], required=True, help="NodeName: should be either e1 through e5 or n1-n2")
    args = parser.parse_args()

    print(f"parser.nodeName: {args.nodeName}")
    if not loadTwoHopTable(args.nodeName):
        sys.exit()

    
    print(f"[+] twoHopTable from JSON: {twoHopTable}\n[+] Distance to e2: {getDistToNode('e2')}, Distance to e1: {getDistToNode('e1')}")
    print(f"[+] distance to non-neighbour e5: {getDistToNode('e5')}")

    
    hbsndthread = HeartbeatSendThread()
    hbrcvthread = HeartbeatReceiveThread()

    print(f"[+] Starting send thread")
    hbsndthread.start()
    print(f"[+] Starting receive thread")
    hbrcvthread.start()

    app.run(host="0.0.0.0", debug=False, use_reloader=False, port=5000)
