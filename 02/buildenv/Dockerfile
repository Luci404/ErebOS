FROM randomdude/gcc-cross-x86_64-elf

RUN apt-get update 
RUN apt-get upgrade -y
# ISO 9660 Rock Ridge Filesystem Manipulator
RUN apt-get install -y xorriso 

VOLUME /root/env
WORKDIR /root/env 