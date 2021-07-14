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

def clear_state():
    req=packetserial.GBPCMD_REQ_CLEAR_STATE;
    print(f"req=[{req}]")
    rep=packetserial.PacketSerialRequest(req)
    print(f"rep=[{rep}]")
    if packetserial.GBPCMD_REP_SUCCESS == rep:
        print("test result good")
        return
    print("test result bad")

def open_and_test():
    packetserial.PacketSerialOpenAndClear()
    time.sleep(1)
    clear_state()
    time.sleep(1)

def main(args):
    print("gamebot clear state")
    open_and_test()

if __name__ == "__main__":
    main(sys.argv)

