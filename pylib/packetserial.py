#!/usr/bin/env python3
#
# Copyright 2021 by angry-kitten
# Serial packet support written for gamebot-serial.
#

import sys
import os
import time
import math
import serial
import zlib

class PacketSerial:

    default_serial_device="/dev/ttyUSB0"
    default_baud=9600

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
    GBPCMD_REQ_MOVE_LEFT_JOY=b'l'
    GBPCMD_REQ_MOVE_RIGHT_JOY=b'r'
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

    SP_START=b'P'
    SP_END=b'E'
    SP_LEN_INVERT=0xf0

    def LowerEightCRC32(self,b):
        v = zlib.crc32(b)
        return v & 0xff

    def RequestPacket(self,s,bs):
        """Write a request packet to the serial line."""
        datalen=len(bs)
        ba=bytearray(bs)
        ba.insert(0,self.SP_START[0])
        l2=datalen|(datalen<<4)
        l2=l2^self.SP_LEN_INVERT
        ba.insert(1,l2)
        ba.append(self.LowerEightCRC32(bs))
        ba.append(self.SP_END[0])
        s.write(ba)
        s.write(bytes('\r\n','utf-8')) # append cr and nl to help with debugging

    def ReplyPacket(self,s):
        """Read a reply packet from the serial line."""
        while True:
            if s.in_waiting <= 0:
                time.sleep(0.01) # sleep 10 milliseconds
            else:
                b=s.read()
                if self.SP_START == b:
                    break
                print("b=",b)
        # The start byte of a packet has been found.
        lb=s.read()
        l=lb[0]
        l=l^self.SP_LEN_INVERT
        l1=l&0x0f
        l2=(l&0xf0)>>4
        if l1 != l2:
            print("bad length")
            return bytes(0)
        databytes=s.read(l1)
        checksum=s.read(1)
        endbyte=s.read(1)
        if self.SP_END[0] != endbyte[0]:
            print("bad end byte")
            return bytes(0)
        crc32ish=self.LowerEightCRC32(databytes)
        if checksum[0] != crc32ish:
            print("bad checksum")
            return bytes(0)
        return databytes

    # req is a bytes or bytearray
    def RequestNoRetry(self,req):
        self.RequestPacket(self.Device,req)
        rep=self.ReplyPacket(self.Device)
        return rep

    # req is a bytes or bytearray
    def Request(self,req):
        for retry in range(3):
            rep=self.RequestNoRetry(req)
            if len(rep) > 0:
                return rep
            print("request failure")
        return rep

    def OpenAndClear(self):
        self.Device=serial.Serial(self.default_serial_device,self.default_baud,timeout=1)
        time.sleep(1) # delay in case the device needs time to get ready
        self.Device.reset_input_buffer() # clear any stale data
        self.Device.reset_output_buffer() # clear any stale data

    def Close(self):
        self.Device.close()

    # request reply functions after here

    def request_query_state(self):
        req=self.GBPCMD_REQ_QUERY_STATE;
        #print(f"req=[{req}]")
        rep=self.Request(req)
        #print(f"rep=[{rep}]")
        if len(rep) != self.GBPCMD_REQ_QUERY_STATE_REPLY_SIZE:
            print("test result bad 2")
            return
        if self.GBPCMD_REQ_QUERY_STATE != rep[0:1]:
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
        #if flags & self.GB_FLAGS_CONFIGURED:
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

    def request_test_alive(self):
        req=self.GBPCMD_REQ_TEST
        #print(f"req=[{req}]")
        rep=self.Request(req)
        #print(f"rep=[{rep}]")
        if len(rep) != 1:
            print("test result bad 2")
            return False
        if self.GBPCMD_REP_ALIVE != rep:
            print("test result bad")
            return False
        #print("test result good")
        return True

    def request_press_buttons(self,buttons,duration_msec):
        req=bytearray(self.GBPCMD_REQ_PRESS_BUTTONS);
        req.append((0xff00&buttons)>>8); # Button high
        req.append(0x00ff&buttons); # Button low
        duration_msec=int(duration_msec)
        if duration_msec > 0:
            req.append((0xff00&duration_msec)>>8);
            req.append(0x00ff&duration_msec);
        #print(f"req=[{req}]")
        rep=self.Request(req)
        #print(f"rep=[{rep}]")
        if self.GBPCMD_REP_SUCCESS != rep:
            print("test result bad")
            return False
        #print("test result good")
        return True

    def request_move_left_joy(self,LX,LY,duration_msec):
        if LX < self.STICK_MIN:
            LX=self.STICK_MIN
        elif LX > self.STICK_MAX:
            LX=self.STICK_MAX
        if LY < self.STICK_MIN:
            LY=self.STICK_MIN
        elif LY > self.STICK_MAX:
            LY=self.STICK_MAX
        req=bytearray(self.GBPCMD_REQ_MOVE_LEFT_JOY);
        req.append(LX) # LX, + is right, - is left
        req.append(LY) # LY, + is down, - is up
        duration_msec=int(duration_msec)
        if duration_msec > 0:
            req.append((0xff00&duration_msec)>>8);
            req.append(0x00ff&duration_msec);
        #print(f"req=[{req}]")
        rep=self.Request(req)
        #print(f"rep=[{rep}]")
        if self.GBPCMD_REP_SUCCESS != rep:
            print("test result bad")
            return False
        #print("test result good")
        return True

    def request_move_right_joy(self,RX,RY,duration_msec):
        if RX < self.STICK_MIN:
            RX=self.STICK_MIN
        elif RX > self.STICK_MAX:
            RX=self.STICK_MAX
        if RY < self.STICK_MIN:
            RY=self.STICK_MIN
        req=bytearray(self.GBPCMD_REQ_MOVE_RIGHT_JOY);
        req.append(RX) # RX, + is right, - is left
        req.append(RY) # RY, + is down, - is up
        duration_msec=int(duration_msec)
        if duration_msec > 0:
            req.append((0xff00&duration_msec)>>8);
            req.append(0x00ff&duration_msec);
        #print(f"req=[{req}]")
        rep=self.Request(req)
        #print(f"rep=[{rep}]")
        if self.GBPCMD_REP_SUCCESS != rep:
            print("test result bad")
            return False
        #print("test result good")
        return True

    def request_press_hat(self,hat,duration_msec):
        req=bytearray(self.GBPCMD_REQ_PRESS_HAT);
        req.append(hat)
        duration_msec=int(duration_msec)
        if duration_msec > 0:
            req.append((0xff00&duration_msec)>>8);
            req.append(0x00ff&duration_msec);
        #print(f"req=[{req}]")
        rep=self.Request(req)
        #print(f"rep=[{rep}]")
        if self.GBPCMD_REP_SUCCESS != rep:
            print("test result bad")
            return False
        #print("test result good")
        return True

    def request_clear_state(self):
        req=self.GBPCMD_REQ_CLEAR_STATE;
        #print(f"req=[{req}]")
        rep=self.Request(req)
        #print(f"rep=[{rep}]")
        if self.GBPCMD_REP_SUCCESS == rep:
            #print("test result good")
            return
        print("test result bad")

    # convenience functions

    # heading= 0=up/north, 90=right/east, 180=down/south, 270=left/west
    # extent= 0.0= 0% nothing/center, 1.0= 100% full/max
    def left_joy_heading(self,heading,extent,duration_msec):
        fsin=math.sin(math.radians(heading)) # + is right
        fcos=math.cos(math.radians(heading)) # + is up
        radius=extent*(self.STICK_MAX-self.STICK_CENTER)
        # dX, + is right, - is left
        # dY, + is down, - is up
        dX=self.STICK_CENTER+int(round(fsin*radius))
        dY=self.STICK_CENTER-int(round(fcos*radius))
        return self.request_move_left_joy(dX,dY,duration_msec)

    # heading= 0=up/north, 90=right/east, 180=down/south, 270=left/west
    # extent= 0.0= 0% nothing/center, 1.0= 100% full/max
    def right_joy_heading(self,heading,extent,duration_msec):
        fsin=math.sin(math.radians(heading)) # + is right
        fcos=math.cos(math.radians(heading)) # + is up
        radius=extent*(self.STICK_MAX-self.STICK_CENTER)
        # dX, + is right, - is left
        # dY, + is down, - is up
        dX=self.STICK_CENTER+int(round(fsin*radius))
        dY=self.STICK_CENTER-int(round(fcos*radius))
        return self.request_move_right_joy(dX,dY,duration_msec)

    def press_Y(self,msec=0):
        return self.request_press_buttons(self.SWITCH_Y,msec)

    def press_B(self,msec=0):
        return self.request_press_buttons(self.SWITCH_B,msec)

    def press_A(self,msec=0):
        return self.request_press_buttons(self.SWITCH_A,msec)

    def press_X(self,msec=0):
        return self.request_press_buttons(self.SWITCH_X,msec)

    def press_L(self,msec=0):
        return self.request_press_buttons(self.SWITCH_L,msec)

    def press_R(self,msec=0):
        return self.request_press_buttons(self.SWITCH_R,msec)

    def press_L_and_R(self,msec=0):
        return self.request_press_buttons(self.SWITCH_L|self.SWITCH_R,msec)

    def press_ZL(self,msec=0):
        return self.request_press_buttons(self.SWITCH_ZL,msec)

    def press_ZR(self,msec=0):
        return self.request_press_buttons(self.SWITCH_ZR,msec)

    def press_MINUS(self,msec=0):
        return self.request_press_buttons(self.SWITCH_MINUS,msec)

    def press_PLUS(self,msec=0):
        return self.request_press_buttons(self.SWITCH_PLUS,msec)

    def press_LCLICK(self,msec=0):
        return self.request_press_buttons(self.SWITCH_LCLICK,msec)

    def press_RCLICK(self,msec=0):
        return self.request_press_buttons(self.SWITCH_RCLICK,msec)

    def press_HOME(self,msec=0):
        return self.request_press_buttons(self.SWITCH_HOME,msec)

    def press_CAPTURE(self,msec=0):
        return self.request_press_buttons(self.SWITCH_CAPTURE,msec)

    def move_left_joy_right(self,msec=0):
        return self.request_move_left_joy(self.STICK_MAX,self.STICK_CENTER,msec)

    def move_left_joy_left(self,msec=0):
        return self.request_move_left_joy(self.STICK_MIN,self.STICK_CENTER,msec)

    def move_left_joy_down(self,msec=0):
        return self.request_move_left_joy(self.STICK_CENTER,self.STICK_MAX,msec)

    def move_left_joy_up(self,msec=0):
        return self.request_move_left_joy(self.STICK_CENTER,self.STICK_MIN,msec)

    def move_right_joy_right(self,msec=0):
        return self.request_move_right_joy(self.STICK_MAX,self.STICK_CENTER,msec)

    def move_right_joy_left(self,msec=0):
        return self.request_move_right_joy(self.STICK_MIN,self.STICK_CENTER,msec)

    def move_right_joy_down(self,msec=0):
        return self.request_move_right_joy(self.STICK_CENTER,self.STICK_MAX,msec)

    def move_right_joy_up(self,msec=0):
        return self.request_move_right_joy(self.STICK_CENTER,self.STICK_MIN,msec)

    def press_hat_TOP(self,msec=0):
        return self.request_press_hat(self.HAT_TOP,msec)

    def press_hat_TOP_RIGHT(self,msec=0):
        return self.request_press_hat(self.HAT_TOP_RIGHT,msec)

    def press_hat_RIGHT(self,msec=0):
        return self.request_press_hat(self.HAT_RIGHT,msec)

    def press_hat_BOTTOM_RIGH(self,msec=0):
        return self.request_press_hat(self.HAT_BOTTOM_RIGH,msec)

    def press_hat_BOTTOM(self,msec=0):
        return self.request_press_hat(self.HAT_BOTTOM,msec)

    def press_hat_BOTTOM_LEFT(self,msec=0):
        return self.request_press_hat(self.HAT_BOTTOM_LEFT,msec)

    def press_hat_LEFT(self,msec=0):
        return self.request_press_hat(self.HAT_LEFT,msec)

    def press_hat_TOP_LEFT(self,msec=0):
        return self.request_press_hat(self.HAT_TOP_LEFT,msec)

    # This doesn't really make any sense since center is the default.
    def press_hat_CENTER(self,msec=0):
        return self.request_press_hat(self.HAT_CENTER,msec)

