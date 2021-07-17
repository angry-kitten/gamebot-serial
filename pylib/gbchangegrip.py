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

# Go to the Change Grip/Order page with the existing
# controllers and then run this to switch to gamebot-serial.

def main(args):
    print("gamebot change grip")

    ps=packetserial.PacketSerial()

    ps.OpenAndClear()

    # L and R may need to be pressed 3+ times
    ps.press_L_and_R()
    time.sleep(1) # second
    ps.press_L_and_R()
    time.sleep(1) # second
    ps.press_L_and_R()
    time.sleep(1) # second
    ps.press_L_and_R()
    time.sleep(1) # second
    ps.press_L_and_R()
    time.sleep(1) # second

    ps.press_A()
    time.sleep(1) # second
    ps.press_B()
    time.sleep(1) # second

    ps.press_hat_LEFT()
    time.sleep(1) # second
    ps.press_hat_LEFT()
    time.sleep(1) # second
    ps.press_hat_LEFT()
    time.sleep(1) # second
    ps.press_hat_LEFT()
    time.sleep(1) # second

    ps.press_hat_TOP()
    time.sleep(1) # second

    ps.press_A()

    ps.Close()

if __name__ == "__main__":
    main(sys.argv)

