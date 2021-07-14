#!/usr/bin/env python3
#
# Copyright 2021 by angry-kitten
# Serial packet support written for gamebot-serial.
# Test that the configuration is working and the device is responding.
#

import sys
import os
import time

import packetserial

# LX, + is right, - is left
# LY, + is down, - is up
# RX, + is right, - is left
# RY, + is down, - is up
# Request and reply data is MSB-first AKA Network Byte Order AKA Big-Endian
# 0       1            2           3    4   5   6   7   8          9
# Prefix, Button high, Button low, Hat, LX, LY, RX, RY, MSec high, MSec low

def press_A():
    req=bytearray(packetserial.GBPCMD_REQ_PRESS_ALL);
    v=packetserial.SWITCH_A
    req.append((0xff00&v)>>8); # Button high
    req.append(0x00ff&v); # Button low
    req.append(0); # Hat
    req.append(packetserial.STICK_CENTER); # LX
    req.append(packetserial.STICK_CENTER); # LY
    req.append(packetserial.STICK_CENTER); # RX
    req.append(packetserial.STICK_CENTER); # RY
    print(f"req=[{req}]")
    rep=packetserial.PacketSerialRequest(req)
    print(f"rep=[{rep}]")
    if packetserial.GBPCMD_REP_SUCCESS == rep:
        print("test result good")
        return
    print("test result bad")

def both_L_and_R():
    req=bytearray(packetserial.GBPCMD_REQ_PRESS_ALL);
    v=packetserial.SWITCH_L|packetserial.SWITCH_R
    req.append((0xff00&v)>>8); # Button high
    req.append(0x00ff&v); # Button low
    req.append(0); # Hat
    req.append(packetserial.STICK_CENTER); # LX
    req.append(packetserial.STICK_CENTER); # LY
    req.append(packetserial.STICK_CENTER); # RX
    req.append(packetserial.STICK_CENTER); # RY
    print(f"req=[{req}]")
    rep=packetserial.PacketSerialRequest(req)
    print(f"rep=[{rep}]")
    if packetserial.GBPCMD_REP_SUCCESS == rep:
        print("test result good")
        return
    print("test result bad")

def move_up():
    req=bytearray(packetserial.GBPCMD_REQ_PRESS_ALL);
    req.append(0); # Button high
    req.append(0); # Button low
    req.append(0); # Hat
    req.append(packetserial.STICK_CENTER); # LX
    req.append(packetserial.STICK_MIN); # LY
    req.append(packetserial.STICK_CENTER); # RX
    req.append(packetserial.STICK_CENTER); # RY
    down_time_msec=1000 # milliseconds AKA 1 second
    req.append(0xff&(down_time_msec>>8))
    req.append(0xff&down_time_msec)
    print(f"req=[{req}]")
    rep=packetserial.PacketSerialRequest(req)
    print(f"rep=[{rep}]")
    if packetserial.GBPCMD_REP_SUCCESS == rep:
        print("test result good")
        return
    print("test result bad")

def open_and_test():
    packetserial.PacketSerialOpenAndClear()
    both_L_and_R()
    time.sleep(1)
    both_L_and_R()
    time.sleep(1)
    both_L_and_R()
    time.sleep(1)
    both_L_and_R()
    time.sleep(1)
    press_A()
    time.sleep(1)
    move_up()

def main(args):
    print("gamebot test move")
    open_and_test()

if __name__ == "__main__":
    main(sys.argv)

