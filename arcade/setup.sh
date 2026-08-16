#!/bin/bash

useradd -m -s /bin/bash arcade
echo "arcade:password" | chpasswd
usermod -aG sudo arcade

mkdir -p /arcade
chown vagrant:vagrant /arcade

apt install -y libopenal1
apt install -y libvorbisfile3

apt install -y xorg libgl1 libglx0 libx11-6 libxrandr2 libxinerama1 libxcursor1 libxi6