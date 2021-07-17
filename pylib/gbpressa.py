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

def open_and_test():
    ps=packetserial.PacketSerial()
    ps.OpenAndClear()
    time.sleep(1)
    ps.press_A()
    time.sleep(1)
    ps.Close()

def main(args):
    print("gamebot press A")
    open_and_test()

if __name__ == "__main__":
    main(sys.argv)

