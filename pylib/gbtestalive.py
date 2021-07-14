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

def do_request_reply_test():
    is_alive=packetserial.request_test_alive()
    if is_alive:
        print("is alive")
    else:
        print("is not alive")

def open_and_test():
    packetserial.PacketSerialOpenAndClear()
    do_request_reply_test();

def main(args):
    print("gamebot test alive")
    open_and_test()

if __name__ == "__main__":
    main(sys.argv)

