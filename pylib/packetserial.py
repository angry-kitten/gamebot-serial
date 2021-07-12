#!/usr/bin/env python3
#
# Copyright 2021 by angry-kitten
# Serial packet support written for gamebot-serial.
#

import sys
import os
import time
import serial
import zlib

default_serial_device="/dev/ttyUSB0"
default_baud=9600

PacketSerialDevice=None

SP_START=b'P'
SP_END=b'E'
SP_LEN_INVERT=0xf0

def LowerEightCRC32(b):
    v = zlib.crc32(b)
    return v & 0xff

def RequestPacket(s,bs):
    datalen=len(bs)
    ba=bytearray(bs)
    ba.insert(0,SP_START[0])
    l2=datalen|(datalen<<4)
    l2=l2^SP_LEN_INVERT
    ba.insert(1,l2)
    ba.append(LowerEightCRC32(bs))
    ba.append(SP_END[0])
    s.write(ba)
    s.write(bytes('\r\n','utf-8')) # append cr and nl to help with debugging

def ReplyPacket(s):
    while True:
        if s.in_waiting <= 0:
            time.sleep(0.01) # sleep 10 milliseconds
        else:
            b=s.read()
            if SP_START == b:
                break
    # The start byte of a packet has been found.
    lb=s.read()
    l=lb[0]
    l=l^SP_LEN_INVERT
    l1=l&0x0f
    l2=(l&0xf0)>>4
    if l1 != l2:
        print("bad length")
        return bytes(0)
    databytes=s.read(l1)
    checksum=s.read(1)
    endbyte=s.read(1)
    if SP_END[0] != endbyte[0]:
        print("bad end byte")
        return bytes(0)
    crc32ish=LowerEightCRC32(databytes)
    if checksum[0] != crc32ish:
        print("bad checksum")
        return bytes(0)
    return databytes

# req is a bytes or bytearray
def PacketSerialRequest(req):
    global PacketSerialDevice
    RequestPacket(PacketSerialDevice,req)
    rep=ReplyPacket(PacketSerialDevice)
    return rep

def PacketSerialOpenAndClear():
    global PacketSerialDevice
    PacketSerialDevice=serial.Serial(default_serial_device,default_baud,timeout=1)
    time.sleep(1) # delay in case the device needs time to get ready
    PacketSerialDevice.reset_input_buffer() # clear any stale data
    PacketSerialDevice.reset_output_buffer() # clear any stale data

