# Tech stack
 - Docker : https://www.docker.com/products/docker-desktop
 - QUMU : https://qemu.weilnetz.de/w64/

# Docs:


# Files
`src/impl/x86_64/boot/header.asm` - implements Multiboot2
`src/impl/x86_64/boot/main.asm` - os entry point


# Build docker environment:
Build an image for the build-environment
 - `docker build buildenv -t khaos-buildenv`

# Run docker environment
Enter build environment:
 - Windows (CMD): `docker run --rm -it -v "%cd%":/root/env khaos-buildenv`
 - Windows (PowerShell): `docker run --rm -it -v "${pwd}:/root/env" khaos-buildenv`

# Build OS
Inside the docker container `make build-x86_64`

To leave the build environment, enter `exit`.

# Emulate
Using Qemu: `qemu-system-x86_64 -cdrom dist/x86_64/kernel.iso`

# Resources
 - https://www.youtube.com/watch?v=FkrpUaGThTQ