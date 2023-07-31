#!/usr/bin/env /Users/atekeoglu/Desktop/learn/docker_tutorial/python-docker/.venv/bin/python
import os, time, sys
from subprocess import check_output as runoutput
from subprocess import run as run
import argparse
from pathlib import Path

NUMHELPERS=4
docker="/usr/local/bin/docker"
VERIFIER="192.168.11.149"
PROVER="192.168.11.106"

def build():
    print(f"[+] About to build the docker image")
    x=runoutput(f"{docker} build --tag python-docker .", shell=True).decode()
    print(f"[+] Trying to delete the lefover image(s) from previous compilation...")
    x=runoutput(f'{docker} images | awk \'{{if($0~/none/)print $3}}\' | xargs -I{{}} docker rmi {{}}', shell=True).decode()
    print(x)
    x=runoutput(f"{docker} images", shell=True).decode()
    print(x)

def stopAll():
    x=runoutput(f"docker ps --format '{{{{.Names}}}}'", shell=True).decode()
    print(f"[+] About to stop the containers named:\n{x}")

    helpers = x.strip().replace("\n", " ")
    print(f"[+] running: {docker} stop {helpers}")
    x=runoutput(f"{docker} stop {helpers}", shell=True).decode()
    print(f"[+] Stopped Containers:\n{x}")


def areContainersUp():
    x=runoutput(f"{docker} ps", shell=True).decode()
    print(x)
    if "helper" in x:
        print("[+] helper container(s) are up...")
        return True
    else:
        print("[-] Couldn't find any containers running...")
        return False


def main():
    if areContainersUp():
        stopAll()
    if args.stopAll:
        exit()
    if args.build:
        build()
        exit()
    if args.numContainers:
        NUMHELPERS = args.numContainers

    for i in range(0, NUMHELPERS):
        print(f"[+] Running docker run ... i={i}")
        x=runoutput(f"{docker} run --rm -itd --network trustflownet \
                    --env VERIFIER={VERIFIER} --env PROVER={PROVER} \
                    --env HOSTNAME=helper_{i} --name helper_{i} --publish {8000+i}:5000 python-docker", shell=True)
        print(f"[+] output: {str(x).strip()}")
    
    time.sleep(1)
    x=runoutput(f"{docker} ps", shell=True).decode()
    print(f"[+] output: {x}")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(add_help=True)
    parser.add_argument("-s", "--stopAll", action="store_true")
    parser.add_argument("-b", "--build", action="store_true")
    parser.add_argument("-n", "--numContainers", type=int, choices=range(1,11))
    args = parser.parse_args()
    main()