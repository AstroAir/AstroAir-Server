AstroAir Server
===============
# Introduction
- Astroair is a new generation of astronomical photography terminal based on the Internet. It adopts Raspberrypi(now we are tyring to build it on Ubuntu) platform and has good software and device compatibility. It can control most of the devices and take pre fast and convenient astronomical photography. Our goal is to make astronomical photography simple.<br>
# Features
- Using GPL3 protocol, is an open source astronomy software, you can modify according to your needs<br>
- With good device support, it can support most devices (through indi) and native ZWO and QHY cameras<br>
- Lightweight, can be deployed on smaller clients, even raspberry pi zero<br>
- Using web page display, support mobile phones, tablets, computers and other devices, no need to download additional software<br>
- High quality GNOME desktop, built-in KStars, SkyChart and other open source software, good stability<br>
# Building
## Install Pre-requisites
On Debian/Ubuntu:
```
sudo apt-get -y install g++ make libnova-dev libcfitsio-dev libusb-1.0-0-dev build-essential cmake git libasio-dev
```
## Get the code
```
git clone https://github.com/AstroAir-Develop-Team/AstroAir-Server.git
```
## Building the server
```
mkdir build&&cd build 
cmake ..
sudo make 
sudo make install #Currently, direct installation to the system is not supported. Please test in the build directory
```
# Problem report
- Join the official QQ group 710622107
- Problem report at GitHub
- Official email<astro_air@126.com>
- Developer email<qwdmax@qq.com>
