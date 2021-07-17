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

# When using gamebot-serial and you want to switch back to
# other conrollers, run this to go to the Change Grip/Order page.

def main(args):
    print("gamebot go to change grip")

    ps=packetserial.PacketSerial()
    ps.OpenAndClear()

    ps.press_HOME()
    time.sleep(1) # second

    ps.press_hat_BOTTOM()
    time.sleep(1) # second

    ps.press_hat_RIGHT()
    time.sleep(1) # second
    ps.press_hat_RIGHT()
    time.sleep(1) # second
    ps.press_hat_RIGHT()
    time.sleep(1) # second
    ps.press_hat_RIGHT()
    time.sleep(1) # second

    ps.press_A()
    time.sleep(1) # second

    ps.press_A()

    ps.Close()


if __name__ == "__main__":
    main(sys.argv)

