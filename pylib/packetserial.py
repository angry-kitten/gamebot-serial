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

# Request and reply data is MSB-first AKA Network Byte Order AKA Big-Endian
# 0       1            2           3    4   5   6   7   8          9
# Prefix, Button high, Button low, Hat, LX, LY, RX, RY, MSec high, MSec low

SWITCH_Y=0x0001
SWITCH_B=0x0002
SWITCH_A=0x0004
SWITCH_X=0x0008
SWITCH_L=0x0010
SWITCH_R=0x0020
SWITCH_ZL=0x0040
SWITCH_ZR=0x0080
SWITCH_MINUS=0x0100
SWITCH_PLUS=0x0200
SWITCH_LCLICK=0x0400
SWITCH_RCLICK=0x0800
SWITCH_HOME=0x1000
SWITCH_CAPTURE=0x2000

HAT_TOP=0x00
HAT_TOP_RIGHT=0x01
HAT_RIGHT=0x02
HAT_BOTTOM_RIGHT=0x03
HAT_BOTTOM=0x04
HAT_BOTTOM_LEFT=0x05
HAT_LEFT=0x06
HAT_TOP_LEFT=0x07
HAT_CENTER=0x08

STICK_MIN=0x00
STICK_CENTER=0x80
STICK_MAX=0xff

# These are the prefixes for the commands in the data
# part of the packets.
GBPCMD_REQ_TEST=b'T'
GBPCMD_REQ_QUERY_STATE=b'Q'
GBPCMD_REQ_DEBUG=b'D'
GBPCMD_REQ_GET_USB_OUT_DATA=b'O' # capital O
GBPCMD_REQ_SET_ALL=b'S'
GBPCMD_REQ_SET_BUTTONS=b'B'
GBPCMD_REQ_SET_LEFT_JOY=b'L'
GBPCMD_REQ_SET_RIGHT_JOY=b'R'
GBPCMD_REQ_SET_HAT=b'H'
GBPCMD_REQ_UNSET_ALL=b'U'
GBPCMD_REQ_SET_DOWN_MSEC=b'M'
GBPCMD_REQ_PRESS_ALL=b's'
GBPCMD_REQ_PRESS_BUTTONS=b'b'
GBPCMD_REQ_PRESS_LEFT_JOY=b'l'
GBPCMD_REQ_PRESS_RIGHT_JOY=b'r'
GBPCMD_REQ_PRESS_HAT=b'h'
GBPCMD_REQ_CLEAR_STATE=b'C'
GBPCMD_REQ_PAUSE_MSEC=b'P'
GBPCMD_REQ_REPORT_PENDING=b'p'

GBPCMD_REP_ALIVE=b'A'
# Define these error numbers as prefix characters so we can have single
# byte responses instead of a prefix plus a number.
GBPCMD_REP_SUCCESS=b'0'  # AKA no error
GBPCMD_REP_ERROR=b'1'
GBPCMD_REP_OVERFLOW=b'2'
GBPCMD_REP_3=b'3'
GBPCMD_REP_4=b'4'
GBPCMD_REP_5=b'5'
GBPCMD_REP_6=b'6'
GBPCMD_REP_7=b'7'
GBPCMD_REP_8=b'8'
GBPCMD_REP_9=b'9'

GBPCMD_REQ_QUERY_STATE_REPLY_SIZE=14
GB_FLAGS_CONFIGURED=0x01

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
            print("b=",b)
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


# request reply functions after here

def request_query_state():
    req=GBPCMD_REQ_QUERY_STATE;
    print(f"req=[{req}]")
    rep=PacketSerialRequest(req)
    print(f"rep=[{rep}]")
    if len(rep) != GBPCMD_REQ_QUERY_STATE_REPLY_SIZE:
        print("test result bad 2")
        return
    if GBPCMD_REQ_QUERY_STATE != rep[0:1]:
        print("test result bad 1")
        return
    print("test result good")
    # decode the data now
    flags=rep[1]
    head=rep[2]
    tail=rep[3]
    count=rep[4]
    ic=rep[5]<<24
    ic|=rep[6]<<16
    ic|=rep[7]<<8
    ic|=rep[8]
    cem=rep[9]<<24
    cem|=rep[10]<<16
    cem|=rep[11]<<8
    cem|=rep[12]
    echo_count=rep[13]

    #print(f"flags={flags}")
    #if flags & GB_FLAGS_CONFIGURED:
    #    print("configured")
    #else:
    #    print("not configured")
    #print(f"head={head}")
    #print(f"tail={tail}")
    #print(f"count={count}")
    #print(f"ic={ic}")
    #print(f"cem={cem}")
    #print(f"echo_count={echo_count}")

    return (flags,head,tail,count,ic,cem,echo_count)


def request_test_alive():
    req=GBPCMD_REQ_TEST
    print(f"req=[{req}]")
    rep=PacketSerialRequest(req)
    print(f"rep=[{rep}]")
    if len(rep) != 1:
        print("test result bad 2")
        return False
    if GBPCMD_REP_ALIVE != rep:
        print("test result bad")
        return False
    print("test result good")
    return True

