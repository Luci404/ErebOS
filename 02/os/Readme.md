# Tech stack
 - Docker : https://www.docker.com/products/docker-desktop
 - QUMU : https://qemu.weilnetz.de/w64/

# How to build:
Build an image for the build-environment: `docker build buildenv -t erebos-buildenv`
Run docker environment Windows (PowerShell): `docker run --rm -it -v "${pwd}:/root/env" khaos-buildenv`
Inside the docker container `make`

# Emulate
Using Qemu: `qemu-system-x86_64 -cdrom image.iso`

# Resources
 - https://wiki.osdev.org/Stivale