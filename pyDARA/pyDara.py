import json, os, socket, sys
import math
import random
import threading
import socket
import time
import datetime
import argparse
from subprocess import check_output as runoutput
from operator import itemgetter

import irobot_create
from math import floor

from flask import Flask, request, jsonify

app = Flask(__name__)

# DARA-1C variables
global node_name
global coord_x
global coord_y
global twoHopTable
global lst_oneHopIPs
global udp_port
global expStartTime         # the FailureDetectionThread will set this timestamp, marking the initial time experiment starts.
global lst_recovery         # everyTime we receive a recovery message, we will append it to this list.
global isiRobot               # Either true or false, based on the -i argument passed. If -i present (true), then node is physically connected to iRobot.

######
# DARA-1C every node needs to run a thread that Broadcasts a message as a heartbeat to it's 1-link neighbours and 
# listens in another thread to incoming heartbeat from 1-link neighbours.
#####
class HeartbeatSendThread(threading.Thread):
    print(f"[+HST+] HeartbeatSendThread object is created...")
    def run(self):
        print("[+HST+] Inside SendThread run...")
        client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        #client.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1) # TODO: Do not Broadcast, send to one-link neighbours.
        client.settimeout(0.2)
        #msg = bytes(f'This is -{os.environ.get("HOSTNAME")}-', 'utf-8')
        msg = bytes(f'This is -{node_name}-', 'utf-8')

        while True:
            #client.sendto(msg, ('<broadcast>', 37020))
            for ip in lst_oneHopIPs:
                client.sendto(msg, (ip, udp_port))
                print(f'[+HST+] H-B-Msg sent to {ip}:{udp_port}...(at {datetime.datetime.now()})')

            time.sleep(5)


class HeartbeatReceiveThread(threading.Thread):
    print(f"[+HRT+] HeartbeatReceiveThread object is created...")
    def run(self):
        print("[+HRT+] Inside ReceiveThread run...")
        client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        #client.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1) # Enable Broadcasting Mode
        client.bind(("", udp_port))
        while True:
            print("[+HRT+] RcvThread: waiting for incoming MSG....")
            data, addr = client.recvfrom(1024)
            rnow = datetime.datetime.now()
            print(f"[+HRT+] Received broadcast MSG: {data} from {addr}. (at {rnow})")
            #TODO:  we need to keep track of the last heart-beat received, maybe in a timestamp format.
            #       check if we can add a field in the twoHopTable["links"]["lastHB"]=datetime.datetime.now()
            for l in twoHopTable["links"]:
                print(f"[+HRT+] link: {l}, addr: {addr[0]}")
                if l["ip"] == str(addr[0]):
                    print(f"[+HRT+] updated last HB received timestamp ({rnow}) for {addr[0]}")
                    l["lastHB"]=str(rnow)
            # TODO: check if someone send a recovery message
            if "recovered" in data:
                # Assume Best Candidate send it in this format
                # {"msg": "recovered", "fn": "eX", "bc": "eY"}
                # TODO: parse message and extract failingnode name
                lst_recovery.append( {"bc": addr[0], "fn": data["fn"], "ts" : datetime.datetime.now()} )


class FailureDetectionThread(threading.Thread):
    global expStartTime
    print(f"[+FDT+] FailureDetectionThread object is created...")
    def run(self):
        failureThreshold = 10        # in seconds
        expStartTime = datetime.datetime.now()
        print(f"[+FDT+] Inside FailureDetectionThread run...expStartTime: {expStartTime}, failureThresholdSeconds: {failureThreshold}")
        
        while True:
            rnow = datetime.datetime.now()
            print(f"[+FDT+] Checking last HB updates from 1-link neighbours: at {rnow}")
            #TODO:  check if this field exists for every neighbour: twoHopTable["links"]["lastHB"]
            #       if ["lastHB" doesn't exist, that means we have not received any heart-beat from this neighbour yet.
            #       if it exists, convert to datetime object, and calculate delta from current time. Print out "last HB received T seconds ago"
            #       if delta is more than 2 * HB_interval or have never received any hb for more than 2 * HB_interval; maybe it is failed?
            for l in twoHopTable["links"]:
                if "lastHB" in l.keys():
                    delta = rnow - datetime.datetime.strptime(l["lastHB"], '%Y-%m-%d %H:%M:%S.%f')
                    print(f"[+FDT+] last HB from {l['name']} was {delta.seconds} seconds ago.")
                    
                else:
                    # TODO: maybe we should add a timestamp for last checked?
                    #       so that, if it has never sent HB for 2 *
                    delta = rnow - expStartTime
                    print(f"[+FDT+] {l['ip']} has never sent HB. Has been {delta.seconds} since start of experiment")
                
                if delta.seconds > failureThreshold:
                    # we assume this neighbour has failed, need to call DARA1C
                    print(f"[+FDT+] delta.seconds ({delta.seconds}) is more than threshold ({failureThreshold}), calling DARA-1C...")
                    dara1C(l["name"], l["ip"], l["coords"])
                    #
            
            time.sleep(5)


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
        lst_recovery = []
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
def dara1C(failedNodeName, failedNodeIP, coordsFailingNode):
    isRecovered=False
    while not isRecovered:
        print(f"[+D1C+] at dara1C for failingNode: {failedNodeName}, {failedNodeIP}")
        if isCutVertex(failedNodeName, failedNodeIP):
            bestNodeName, bestNodeIP, distance = findBestCandidate(failedNodeName, failedNodeIP)
            
            if bestNodeName in twoHopTable["name"] and bestNodeIP in twoHopTable["ip"]:
                print(f"[+D1C+] We are the best candidate, MOVING to replace failing node {failedNodeName}")
                moveToLocation(coordsFailingNode, distance)
                #TODO: do we need a different updateTwoHopTable for the BestCandidate????
                updateTwoHopTable(failedNodeName, bestNodeName)
                #TODO: 
                sendRevoredMSG(failedNodeName, bestNodeName)

            else:
                # TODO: you are not the best candidate, you need to calculate how much time it would take the Best Candidate to replace; (t)
                #       update the table (replace the deaed with best candidate)
                #       wait 2*t for BC to send an update message that he reached. 
                #       If updateReceived:
                #           isRecovered=True
                #       else if you dont' receive update in 2*t,
                #           isRecovered=False
                #           
                print(f"[+D1C+] We are NOT the best candidate. {bestNodeName} is moving {distance} units to replace")
                updateTwoHopTable(failedNodeName, bestNodeName)
                t = calcTimeToWaitForReplacement( coordsFailingNode, getCoords(bestNodeName) )
                print("[+] Time to wait")
                
                startTime=datetime.datetime.now()
                time.sleep(2*t)
                endTime=datetime.datetime.now()

                if not isRecoveredMSGReceived(startTime, endTime, bestNodeName, failedNodeName):
                    failedNodeName = bestNodeName
                    failedNodeIP = bestNodeIP
                    dara1C(failedNodeName, failedNodeIP, coordsFailingNode)

    return

# TODO: Should send a unicast message to all the (new) one-link neighbours. Assuming that twoHopTable of the BC is updated prior.
def sendRevoredMSG(failName, bestName):
        print("[+SRM+] Inside Send Recovery Message...")
        client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        #client.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1) # TODO: Do not Broadcast, send to one-link neighbours.
        client.settimeout(0.2)
        #msg = bytes(f'This is -{os.environ.get("HOSTNAME")}-', 'utf-8')
        # msg format: {"msg": "recovered", "fn": "eX", "bc": "eY"}
        msg_json=f'{{"msg":"recovered","fn":{failName},"bc":{bestName}}}'
        msg = bytes(f"{msg_json}", 'utf-8')

        while True:
            #client.sendto(msg, ('<broadcast>', 37020))
            lst_oneHopIPs = getOneHopNeighbourIPs()
            for ip in lst_oneHopIPs:
                client.sendto(msg, (ip, udp_port))
                print(f'[+HST+] H-B-Msg sent to {ip}:{udp_port}...(at {datetime.datetime.now()})')


# TODO:
def isRecoveredMSGReceived(startTime, endTime, bnode, fnode):
    for i in lst_recovery:
        if i["ts"] >= startTime and i["ts"] < endTime:
            if fnode in i["fn"] and bnode in i["bc"]:
                return True
            
    return False

# TODO:
def calcTimeToWaitForReplacement(coordF, coordB):
    total_time=0

    f_x = float(coordF.split(',')[0])
    f_y = float(coordF.split(',')[1])

    b_x = float(coordB.split(',')[0])
    b_y = float(coordB.split(',')[1])

    time_to_turn_90_degrees = 1
    time_to_travel_one_tile = 2

    # Turning time
    if f_x < b_x:
        total_time += time_to_turn_90_degrees * 2 
        if f_y < b_y:
            total_time += time_to_turn_90_degrees * 3
            total_time += time_to_turn_90_degrees * 3   # after it reaches, it should face x + direction.
        elif f_y == b_y:
            total_time += time_to_turn_90_degrees * 2   # after it reaches, it should face x + direction.
        elif f_y > b_y:
            total_time += time_to_turn_90_degrees * 1
            total_time += time_to_turn_90_degrees * 1   # after it reaches, it should face x + direction.
    elif f_x == b_x:
        if f_y < b_y:
            total_time += time_to_turn_90_degrees * 1   # to turn upwards
            total_time += time_to_turn_90_degrees * 3   # after it reaches, it should face x + direction.
        elif  f_y > b_y:
            total_time += time_to_turn_90_degrees * 3   # to turn downwards
            total_time += time_to_turn_90_degrees * 1   # after it reaches, it should face x + direction.
    elif f_x > b_x:
        if f_y < b_y:
            total_time += time_to_turn_90_degrees * 1   # to turn upwards
            total_time += time_to_turn_90_degrees * 3   # after it reaches, it should face x + direction.
        #elif f_y == b_y:
            # no turning
        elif f_y > b_y:
            total_time += time_to_turn_90_degrees * 3   # to turn downwards
            total_time += time_to_turn_90_degrees * 1   # after it reaches, it should face x + direction.

    # driving time
    total_time += time_to_travel_one_tile *  ( abs(f_x - b_x) + abs(f_y - b_y) ) 
    return total_time

# TODO: 
def getCoords(nodeName):
    if nodeName in twoHopTable["name"]:
        return twoHopTable["coords"]
    
    for n in twoHopTable["link"]:
        if nodeName in n["name"]:
            return n["coords"]
        for nn in n["links"]:
            if nodeName in nn["name"]:
                return nn["coords"]

""" TODO:   Double check with Dr Younis, how to find if a node is a cut-vertex from your twoHopTable."""
def isCutVertex(fName, fIP):
    for l in twoHopTable["links"]:
        if fName in l["name"] and fIP in l["ip"]:
            if int(l["numLinks"]) > 2:
                print("[+ICV+] failing node is a CUT-VERTEX, continue with DARA-1C")
                return True
            else:
                print("[+ICV+] failing node is NOT a CUT-VERTEX (degree less than 3), no need to replace it.")
                return False


def findBestCandidate(fName, fIP):
    print(f"[+FBC+] FindBestCandidate({fName}, {fIP})")
    best={}
    for l in twoHopTable["links"]:
        if fName in l["name"] and fIP in l["ip"]:
            print(f"[+FBC+] links of the failed node: {l['links']}")
            coords_failing_node = l["coords"]
            
            #TODO: for some-reason dstToFailing element additons persist in the twoHopTable, make sure they don't!!
            #TODO: try deep-copy of each dictionary in the list
            # from copy import deepcopy
            # myCopy = deepcopy(myDict)
            print(f"[+FBC+] Coords of failingNode: {coords_failing_node}")
            ptr = l["links"]
            lst_candidates = ptr[:]    # making a copy of the list, so that original twoHopTable doesn't change.
            lst_candidates.append( {"name": node_name, "ip":twoHopTable["ip"], "coords": twoHopTable["coords"], "numLinks": twoHopTable["numLinks"]} )
            # TODO: add yourself (this current node to the lst_candidates[]) [+DONE+]
            for c in lst_candidates:
                c["dstToFailing"] = dist( coords_failing_node, c["coords"] )

            print(f"[+FBC+] Best candidates: {lst_candidates}")
            lst_candidates = sorted(lst_candidates, key=itemgetter('dstToFailing')) # sort by distance to failing node.
            lst_candidates = sorted(lst_candidates, key=itemgetter('numLinks'))    # asending sort by node degree.
            
            best = lst_candidates[0]
    print(f"[+FBC+] sorted_candidates: {lst_candidates}. First index is the best candidate.")
    return best["name"], best["ip"], best["dstToFailing"]  # I Need the coordinates of best candidate and failed node

""" TODO:   This will be calling the iRobot script to move the iRobot into that coordinates"""
#def moveToLocation(coords, distance):
def moveToLocation(fcoords, distance):
    source=[]
    destination=[]
    print(f'[+MTL+] iRobot carrying me: {node_name} is moving {distance} units from {twoHopTable["coords"]}to Coordinates: {fcoords} to replace failing node.')
    source.append(float(twoHopTable["coords"].split(",")[0]))
    source.append(float(twoHopTable["coords"].split(",")[1]))
    destination.append(float(fcoords.split(",")[0]))
    destination.append(float(fcoords.split(",")[1]))

    # NOTE: If --iRobot or -i argument provided, then we have a serial/usb connection to a real iRobot.
    if isiRobot:
        roomba = irobot_create.Roomba('/dev/ttyUSB0')

        x_diff=destination[0]-source[0]
        y_diff=destination[1]-source[1]

        unitfront=1.575 #Time related to unit distance in grid
        halffront=0.7875  #Time related to half unit distance in grid
        qautspin=1.05    #90 degree spin towards left

        print(f"[+MTL+] Moving in X axis...")
        if x_diff>0:
            for i in range(floor(x_diff)):
                roomba.set_drive_straight()
                time.sleep(unitfront) 
                roomba.set_drive_stop()
                time.sleep(0.5)
            if (x_diff%1)!=0:
                roomba.set_drive_straight()
                time.sleep(halffront)
                roomba.set_drive_stop()
                time.sleep(0.5)

        elif x_diff<0:

            roomba.set_drive_spin_cw()
            time.sleep(qautspin)
            roomba.set_drive_stop()
            time.sleep(0.5)
            
            roomba.set_drive_spin_cw()
            time.sleep(qautspin)
            roomba.set_drive_stop()
            time.sleep(0.5)

            for i in range(floor(abs(x_diff))):
                roomba.set_drive_straight()
                time.sleep(unitfront)
                roomba.set_drive_stop()
                time.sleep(0.5)
            if (abs(x_diff)%1)!=0:
                roomba.set_drive_straight()
                time.sleep(halffront)
                roomba.set_drive_stop()
                time.sleep(0.5)
                
            roomba.set_drive_spin_cw()
            time.sleep(qautspin)
            roomba.set_drive_stop()
            time.sleep(0.5)
            
            roomba.set_drive_spin_cw()
            time.sleep(qautspin)
            roomba.set_drive_stop()
            time.sleep(0.5)

        print(f"[+MTL+] Moving in Y axis...")
        if y_diff>0:
            
            #three left turns to go to positive y 
            
            roomba.set_drive_spin_cw()
            time.sleep(qautspin)
            roomba.set_drive_stop()
            time.sleep(0.5)
            
            roomba.set_drive_spin_cw()
            time.sleep(qautspin)
            roomba.set_drive_stop()
            time.sleep(0.5)
            
            roomba.set_drive_spin_cw()
            time.sleep(qautspin)
            roomba.set_drive_stop()
            time.sleep(0.5)
            
            for i in range(floor(y_diff)):
                roomba.set_drive_straight()
                time.sleep(unitfront)
                roomba.set_drive_stop()
                time.sleep(0.5)
            if (y_diff%1)!=0:
                roomba.set_drive_straight()
                time.sleep(halffront)
                roomba.set_drive_stop()
                time.sleep(0.5)
            
            roomba.set_drive_spin_cw()
            time.sleep(qautspin)
            roomba.set_drive_stop()
            time.sleep(0.5)
                
        elif y_diff<0:
            
            roomba.set_drive_spin_cw()
            time.sleep(qautspin)
            roomba.set_drive_stop()
            time.sleep(0.5)
            
            for i in range(floor(abs(y_diff))):
                roomba.set_drive_straight()
                time.sleep(unitfront)
                roomba.set_drive_stop()
                time.sleep(0.5)
            if (abs(y_diff)%1)!=0:
                roomba.set_drive_straight()
                time.sleep(halffront)
                roomba.set_drive_stop()
                time.sleep(0.5)
                
            roomba.set_drive_spin_cw()
            time.sleep(qautspin)
            roomba.set_drive_stop()
            time.sleep(0.5)
            
            roomba.set_drive_spin_cw()
            time.sleep(qautspin)
            roomba.set_drive_stop()
            time.sleep(0.5)
            
            roomba.set_drive_spin_cw()
            time.sleep(qautspin)
            roomba.set_drive_stop()
            time.sleep(0.5)

        print(f"[+MTL+] Roomba should be at the destination now...")
        roomba.close()

    # NOTE: otherwise, we are going to calculate how long it would take a real iRobot to travel from 
    #       bestCandidate coords to failingNode coords, and sleep that much.
    else:
        t = calcTimeToWaitForReplacement(fcoords, twoHopTable["coords"])
        print(f"[+] Simulated iRobot is driving towards failing node... will take {t} seconds")
        time.sleep(t)
    
    return


""" TODO:   If we move, or a neighbour moves, we need to update the twoHopTable to reflect the new topology"""
def updateTwoHopTable(deadNode, replacementNode):
    for l in twoHopTable["links"]:
        if deadNode in l["name"]:
            lst_deadNode = l
        index_replacement=0
        index=0
        for ll in l["links"]:
            if replacementNode in ll["name"]:
                lst_replacementNode = ll
                index_replacement=index
            index+=1

    lst_deadNode["name"] = lst_replacementNode["name"]
    lst_deadNode["ip"] = lst_replacementNode["ip"]
    lst_deadNode["coords"]
    lst_deadNode["numLinks"] = str(int(lst_deadNode["numLinks"])-1)
    del lst_deadNode["links"][index_replacement]
    return

""" TODO:   If our parent moves, we need to run this function."""
def childMoveOptimizer():
    return

""" TODO:   Calculate euclidian distance given 2 coordinates. c1&c2 are in string format in the form of 'x,y' """
def dist(c1, c2):
    c1x=float(c1.split(',')[0])
    c1y=float(c1.split(',')[1])

    c2x=float(c2.split(',')[0])
    c2y=float(c2.split(',')[1])

    return math.sqrt((c1x - c2x)**2 + (c1y - c2y)**2)   #distance btwn c1 and c2

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
    parser.add_argument("-i", "--iRobot", action="store_true", required=False, help="If flag is provided, we assume node has a real iRobot. If not provided, iRobot is simulated.")
    args = parser.parse_args()

    print(f"parser.nodeName: {args.nodeName}")
    if not loadTwoHopTable(args.nodeName):
        sys.exit()

    if args.iRobot:
        isiRobot = True
    else:
        isiRobot = False

    #sys.exit()
    
    print(f"[+] twoHopTable from JSON: {twoHopTable}\n[+] Distance to e2: {getDistToNode('e2')}, Distance to e1: {getDistToNode('e1')}")
    print(f"[+] distance to non-neighbour e5: {getDistToNode('e5')}")

    
    hbSndthread = HeartbeatSendThread()
    hbRcvThread = HeartbeatReceiveThread()
    failDetThread = FailureDetectionThread()

    print(f"[+] Starting send thread")
    hbSndthread.start()
    print(f"[+] Starting receive thread")
    hbRcvThread.start()
    print(f"[+] Starting failure detection thread")
    failDetThread.start()

    app.run(host="0.0.0.0", debug=False, use_reloader=False, port=5000)
