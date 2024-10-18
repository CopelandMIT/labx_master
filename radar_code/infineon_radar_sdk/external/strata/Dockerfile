#FROM ubuntu:16.04
#RUN apt-get update
#RUN apt-get -y install git gcc g++ cmake python3-dev

###

FROM ubuntu_build_tools

COPY . /strata
WORKDIR /strata

SHELL ["/bin/bash", "-c"]
RUN ./build.sh

