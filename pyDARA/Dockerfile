# syntax=docker/dockerfile:1
FROM python:3.8-slim-buster

WORKDIR /app

COPY requirements.txt requirements.txt
RUN pip3 install -r requirements.txt
RUN DEBIAN_FRONTEND='noninteractive' \
    apt update -y && apt-get upgrade -y && \
    apt install -y iputils-ping net-tools curl

COPY . .

CMD [ "python3", "-m", "flask", "run", "--host=0.0.0.0" ]

# At this point we can build the image based on this Docker file by running this command:
#$ docker build --tag python-docker .

# list images:
#$ docker images

# Tag images:
#$ docker tag python-docker:latest python-docker:v1.0.0

# To remove the tag (not the image itself)
#$ docker rmi python-docker:v1.0.0

# To run the image;
#$ docker run python-docker

# To run a docker container in detached mode, port mapped to 
#$ docker run -d -p 8000:5000 --name rest-server python-docker

# If docker has issues, you can stop it via
#$ killall Docker && open /Applications/Docker.app

# TRUSTFLOW NETWORK USER DEFINED BRIDGE
#$ docker network create trustflownet

# TO RUN (start a container from our python-docker image)
#$ docker run --rm -itd --network trustflownet --name helper_i --publish 8000+i:5000 python-docker

# TO GET INTO A SHELL OF A RUNNING CONTAINER
#$ docker exec -it helper_i bash

# TO LIST NETWORKS
#$ docker network list

# TO INSPECT A NETWORK NAMED TRUSTFLOWNET
