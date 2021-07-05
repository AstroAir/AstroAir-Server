AstroAir Server
===============
[![Build Status](https://app.travis-ci.com/AstroAir-Develop-Team/AstroAir-Server.svg?branch=master)](https://app.travis-ci.com/AstroAir-Develop-Team/AstroAir-Server)
[![CircleCI](https://circleci.com/gh/AstroAir-Develop-Team/AstroAir-Server/tree/master.svg?style=svg)](https://circleci.com/gh/AstroAir-Develop-Team/AstroAir-Server/tree/master)
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
### On Debian/Ubuntu:
```
sudo apt-get -y install make libnova-dev libcfitsio-dev libusb-1.0-0-dev build-essential cmake git libasio-dev libwebsocketpp-dev libssl-dev libccfits-dev libboost-dev libopencv-dev libgsl-dev
```
And you alse need to install all of the SDK.
### Use Clang OR GCC
```
sudo apt install clang-11  OR  sudo apt install gcc
```
### PHD2
Guiding depends on PHD2,so you are supposed to install PHD2
```
sudo add-apt-repository ppa:pch/phd2
sudo apt install phd2
```
### Bug Fix
For some unkownn reason,we can use opencv proprely.We need it in "/usr/include/opencv2",but it will be in "/usr/include/opencv4/opencv2".To solve this,you should run:
```
sudo mv /usr/include/opencv4/opencv2 /usr/include
```

## Get the code
```
git clone https://github.com/AstroAir-Develop-Team/AstroAir-Server.git
Or
git clone https://github.com/AstroAir/AstroAir-Server.git   (This port doesn't update on time!)
```
## Building the server
```
mkdir build&&cd build 
cmake ..
make -j4        #This depends on your computer
sudo make install #Currently, direct installation to the system is not supported. Please test in the build directory
```
## Test
```
./airserver     #In building directory
```
# Problem report
- Join the official QQ group 710622107
- Problem report at GitHub
- Official email<astro_air@126.com>
- Developer email<qwdmax@qq.com>
