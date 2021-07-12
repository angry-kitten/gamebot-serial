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

GBPCMD_REQ_TEST=b'T'
GBPCMD_REP_ERROR=b'E'
GBPCMD_REP_ALIVE=b'A'


def send_test_request():
    req=GBPCMD_REQ_TEST
    print(f"req=[{req}]")
    rep=packetserial.PacketSerialRequest(req)
    print(f"rep=[{rep}]")
    if GBPCMD_REP_ALIVE == rep:
        print("test result good")
        return
    print("test result bad")

def do_request_reply_test():
    send_test_request()

def open_and_test():
    packetserial.PacketSerialOpenAndClear()
    do_request_reply_test();

def main(args):
    print("gamebot test alive")
    open_and_test()

if __name__ == "__main__":
    main(sys.argv)

