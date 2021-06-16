FROM randomdude/gcc-cross-x86_64-elf

RUN apt-get update 
RUN apt-get upgrade -y
# The Netwide Assembler is an assembler and disassembler for the Intel x86 architecture.
RUN apt-get install -y nasm
# Needed for grub to work
RUN apt-get install -y xorriso
# GNU GRUB is a boot loader package from the GNU Project
RUN apt-get install -y grub-pc-bin
RUN apt-get install -y grub-common

# Hook up volume to development environment
VOLUME /root/env
# Set startup folder
WORKDIR /root/env 