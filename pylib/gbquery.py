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

def calculate_delta(c1,c2):
    if c1 <= c2:
        return c2-c1
    return (c2+(1<<32))-c1

def calculate_rate(c1,c2,seconds):
    d=calculate_delta(c1,c2)
    r=d/seconds
    return r

def query_state():
    test_time_sec=10

    (flags,head,tail,count,ic,cem,echo_count)=packetserial.request_query_state()
    print(f"flags={flags}")
    if flags & packetserial.GB_FLAGS_CONFIGURED:
        print("configured")
    else:
        print("not configured")
    print(f"head={head}")
    print(f"tail={tail}")
    print(f"count={count}")
    print(f"ic={ic}")
    print(f"cem={cem}")
    print(f"echo_count={echo_count}")

    ic1=ic
    cem1=cem

    time.sleep(test_time_sec)

    (flags,head,tail,count,ic,cem,echo_count)=packetserial.request_query_state()
    print(f"flags={flags}")
    if flags & packetserial.GB_FLAGS_CONFIGURED:
        print("configured")
    else:
        print("not configured")
    print(f"head={head}")
    print(f"tail={tail}")
    print(f"count={count}")
    print(f"ic={ic}")
    print(f"cem={cem}")
    print(f"echo_count={echo_count}")

    ic2=ic
    cem2=cem

    print("interrupt rate",calculate_rate(ic1,ic2,test_time_sec),"per second")
    print("command elapsed rate",calculate_rate(cem1,cem2,test_time_sec),"per second")


def open_and_test():
    packetserial.PacketSerialOpenAndClear()
    time.sleep(1)
    query_state()

def main(args):
    print("gamebot query state")
    open_and_test()

if __name__ == "__main__":
    main(sys.argv)

