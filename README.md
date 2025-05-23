# AsterCTI
Asterisk Computer-Telephone Integration Application with GUI on wxWidgets.

Currently can be used for displaying Caller ID and originating calls.
Originating can be done by executing `astercti tel:<number>`

Main window:   
![Main window screenshot](/screenshot.png?raw=true)

Incoming call notification:  
![Notification screenshot](/screenshot-notify.png?raw=true)

Originating a call:  
![Originate screenshot](/screenshot-originate.png?raw=true)

Installation
------------

There are packages for Debian:

```
deb [arch=amd64] https://yohanson.github.io/debian bullseye main
deb [arch=amd64] https://yohanson.github.io/debian bookworm main
```

Installing the prebuilt package in Debian:

```
wget -O- https://yohanson.github.io/debian/pubkey.gpg | sudo tee /etc/apt/trusted.gpg.d/astercti.asc
echo "deb [arch=amd64] https://yohanson.github.io/debian $(lsb_release -sc) main" | sudo tee /etc/apt/sources.list.d/astercti.list
sudo apt update
sudo apt install astercti
```

Same without sudo:

```
wget -O- https://yohanson.github.io/debian/pubkey.gpg > /etc/apt/trusted.gpg.d/astercti.asc
echo "deb [arch=amd64] https://yohanson.github.io/debian $(lsb_release -sc) main" > /etc/apt/sources.list.d/astercti.list
apt update
apt install astercti
```
