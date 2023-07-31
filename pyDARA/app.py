import json, os, socket, sys
import math
import random
import threading
import socket
import time
import datetime
from subprocess import check_output as runoutput

from flask import Flask, request, jsonify

app = Flask(__name__)

# Challenge_rx=[]
# helper_id=[]

# DARA-1C variables
node_name = ""
# coordinates (x, y)
coord_x = 0
coord_y = 0
# oneHopTable = { "IP": "", "name": ""}
twoHopTable = { "IP": "", "name": "", "coord": ("", ""), "nextHop": { "IP": "", "name": ""} }

######
# DARA-1C every node need to run a thread that Broadcasts a message as a heartbeat and 
# listens in another thread to incoming heartbeat from neighbours.
#####
class HeartbeatSendThread(threading.Thread):
    print(f"[+] HeartbeatSendThread object is created...")
    def run(self):
        print("[+] Inside SendThread run...")
        client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        client.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1) # Enable Broadcasting Mode
        client.settimeout(0.2)
        msg = bytes(f'This is -{os.environ.get("HOSTNAME")}-', 'utf-8')

        while True:
            client.sendto(msg, ('<broadcast>', 37020))
            print(f'[+] Msg sent as broadcast...(at {datetime.datetime.now()})')
            time.sleep(5)

class HeartbeatReceiveThread(threading.Thread):
    print(f"[+] HeartbeatReceiveThread object is created...")
    def run(self):
        print("[+] Inside ReceiveThread run...")
        client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        client.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1) # Enable Broadcasting Mode
        client.bind(("", 37020))
        while True:
            print("[+] RcvThread: waiting for incoming MSG....")
            data, addr = client.recvfrom(1024)
            print(f"[+] Received broadcast MSG: {data} from {addr}. (at {datetime.datetime.now()})")


######
# DARA-1C functions
#####
def getFailedNeighbour():
    failedNeighbours=[]
    return failedNeighbours



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

    hbsndthread = HeartbeatSendThread()
    hbrcvthread = HeartbeatReceiveThread()

    print(f"[+] Starting send thread")
    hbsndthread.start()
    print(f"[+] Starting receive thread")
    hbrcvthread.start()

    app.run(host="0.0.0.0", debug=False, use_reloader=False, port=5000)