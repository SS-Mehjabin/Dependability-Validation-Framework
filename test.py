import argparse


parser = argparse.ArgumentParser()
parser.add_argument("-i", "--iRobot", action="store_true",required=False, help="If flag is provided, we assume node has a real iRobot. If not provided, iRobot is simulated.")

args = parser.parse_args()

print(f"[+] args.iRobot: -{args.iRobot}-")
