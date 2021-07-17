/*
Copyright 2021 by angry-kitten
Serial packet support written for gamebot-serial.
*/

#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Drivers/Peripheral/Serial.h>
#include <LUFA/Platform/Platform.h>

#include "gamebotserial.h"
#include "packetserial.h"
#include "cmdqueue.h"

uint16_t default_press_duration_msec=DEFAULT_BUTTON_PRESS_DURATION;

void ReplyByte(uint8_t b)
{
    ReplyPacket(&b,1);
}

void ReplySuccess(void)
{
    ReplyByte(GBPCMD_REP_SUCCESS);
}

void ReplyError(void)
{
    ReplyByte(GBPCMD_REP_ERROR);
}

void ReplyAlive(void)
{
    ReplyByte(GBPCMD_REP_ALIVE);
}

void ReplyOverflow(void)
{
    ReplyByte(GBPCMD_REP_OVERFLOW);
}


void RequestQueryState(uint8_t *rp, uint8_t rl)
{
    uint8_t reply[GBPCMD_REQ_QUERY_STATE_REPLY_SIZE];

    memset(reply,0,sizeof(reply));

    reply[0]=GBPCMD_REQ_QUERY_STATE;

    if( USB_DeviceState == DEVICE_STATE_Configured )
    {
        reply[1]|=GB_FLAGS_CONFIGURED;
    }

    reply[2]=cmdq.head;
    reply[3]=cmdq.tail;
    reply[4]=cmdq.count;
    uint32_t ic=interrupt_count;
    reply[5]=0xff&(ic>>24);
    reply[6]=0xff&(ic>>16);
    reply[7]=0xff&(ic>>8);
    reply[8]=0xff&ic;
    uint32_t cem=cmd_elapsed_msec;
    reply[9]=0xff&(cem>>24);
    reply[10]=0xff&(cem>>16);
    reply[11]=0xff&(cem>>8);
    reply[12]=0xff&cem;
    reply[13]=echo_count;

    ReplyPacket(reply,sizeof(reply));
}

void RequestDebug(uint8_t *rp, uint8_t rl)
{
    ReplyError();
}

void RequestGetUSBOutData(uint8_t *rp, uint8_t rl)
{
    ReplyError();
}

void RequestSetAll(uint8_t *rp, uint8_t rl)
{
    ReplyError();
}

void RequestSetButtons(uint8_t *rp, uint8_t rl)
{
    ReplyError();
}

void RequestSetLeftJoy(uint8_t *rp, uint8_t rl)
{
    ReplyError();
}

void RequestSetRightJoy(uint8_t *rp, uint8_t rl)
{
    ReplyError();
}

void RequestSetHat(uint8_t *rp, uint8_t rl)
{
    ReplyError();
}

void RequestUnsetAll(uint8_t *rp, uint8_t rl)
{
    ReplyError();
}

void RequestSetDownMSec(uint8_t *rp, uint8_t rl)
{
    ReplyError();
}

void RequestPressAll(uint8_t *rp, uint8_t rl)
{
    if( rl < 8 )
    {
        ReplyError();
    }
    if( rl > 10 )
    {
        ReplyError();
    }

    uint8_t f=CMDQueueFree();
    if( f < 2 )
    {
        ReplyOverflow();
        return;
    }

    // Set up the down strokes.
    cmdqueue_element_t *pe=NULL;
    CMDQueueAdd(&pe);
    if( ! pe )
    {
        ReplyOverflow();
        return;
    }

    // LX, + is right, - is left
    // LY, + is down, - is up
    // RX, + is right, - is left
    // RY, + is down, - is up
    // Request and reply data is MSB-first AKA Network Byte Order AKA Big-Endian
    // 0       1            2           3    4   5   6   7   8          9
    // Prefix, Button high, Button low, Hat, LX, LY, RX, RY, MSec high, MSec low

    pe->i.Button = rp[1]<<8;
    pe->i.Button |= rp[2];
    pe->i.HAT = rp[3];
    pe->i.LX = rp[4];
    pe->i.LY = rp[5];
    pe->i.RX = rp[6];
    pe->i.RY = rp[7];

    if( rl >= 9 )
    {
        pe->duration_msec = rp[8]<<8;
        if( rl >= 10 )
        {
            pe->duration_msec |= rp[9];
        }
    }
    else
    {
        pe->duration_msec=default_press_duration_msec;
    }

    // Set up the up strokes.
    // Since this is a full button press, we now need
    // to release the buttons.

    pe=NULL;
    CMDQueueAdd(&pe);
    if( ! pe )
    {
        // This shouldn't happen.
        ReplyOverflow();
        return;
    }

    // It's already initialized to its default state so
    // we don't have to do anything.

    ReplySuccess();
}

void RequestPressButtons(uint8_t *rp, uint8_t rl)
{
    if( rl < 3 )
    {
        ReplyError();
    }
    if( rl > 5 )
    {
        ReplyError();
    }

    uint8_t f=CMDQueueFree();
    if( f < 2 )
    {
        ReplyOverflow();
        return;
    }

    // Set up the down strokes.
    cmdqueue_element_t *pe=NULL;
    CMDQueueAdd(&pe);
    if( ! pe )
    {
        ReplyOverflow();
        return;
    }

    // Request and reply data is MSB-first AKA Network Byte Order AKA Big-Endian
    // 0       1            2           3          4
    // Prefix, Button high, Button low, MSec high, MSec low

    pe->i.Button = rp[1]<<8;
    pe->i.Button |= rp[2];

    if( rl >= 4 )
    {
        pe->duration_msec = rp[3]<<8;
        if( rl >= 5 )
        {
            pe->duration_msec |= rp[4];
        }
    }
    else
    {
        pe->duration_msec=default_press_duration_msec;
    }

    // Set up the up strokes.
    // Since this is a full button press, we now need
    // to release the buttons.

    pe=NULL;
    CMDQueueAdd(&pe);
    if( ! pe )
    {
        // This shouldn't happen.
        ReplyOverflow();
        return;
    }

    // It's already initialized to its default state so
    // we don't have to do anything.

    ReplySuccess();
}

void RequestMoveLeftJoy(uint8_t *rp, uint8_t rl)
{
    if( rl < 3 )
    {
        ReplyError();
    }
    if( rl > 5 )
    {
        ReplyError();
    }

    uint8_t f=CMDQueueFree();
    if( f < 2 )
    {
        ReplyOverflow();
        return;
    }

    // Set up the down strokes.
    cmdqueue_element_t *pe=NULL;
    CMDQueueAdd(&pe);
    if( ! pe )
    {
        ReplyOverflow();
        return;
    }

    // LX, + is right, - is left
    // LY, + is down, - is up
    // Request and reply data is MSB-first AKA Network Byte Order AKA Big-Endian
    // 0       1   2   3          4
    // Prefix, LX, LY, MSec high, MSec low

    pe->i.LX = rp[1];
    pe->i.LY = rp[2];

    if( rl >= 4 )
    {
        pe->duration_msec = rp[3]<<8;
        if( rl >= 5 )
        {
            pe->duration_msec |= rp[4];
        }
    }
    else
    {
        pe->duration_msec=default_press_duration_msec;
    }

    // Set up the up strokes.
    // Since this is a full button press, we now need
    // to release the buttons.

    pe=NULL;
    CMDQueueAdd(&pe);
    if( ! pe )
    {
        // This shouldn't happen.
        ReplyOverflow();
        return;
    }

    // It's already initialized to its default state so
    // we don't have to do anything.

    ReplySuccess();
}

void RequestMoveRightJoy(uint8_t *rp, uint8_t rl)
{
    if( rl < 3 )
    {
        ReplyError();
    }
    if( rl > 5 )
    {
        ReplyError();
    }

    uint8_t f=CMDQueueFree();
    if( f < 2 )
    {
        ReplyOverflow();
        return;
    }

    // Set up the down strokes.
    cmdqueue_element_t *pe=NULL;
    CMDQueueAdd(&pe);
    if( ! pe )
    {
        ReplyOverflow();
        return;
    }

    // RX, + is right, - is left
    // RY, + is down, - is up
    // Request and reply data is MSB-first AKA Network Byte Order AKA Big-Endian
    // 0       1   2   3          4
    // Prefix, RX, RY, MSec high, MSec low

    pe->i.RX = rp[1];
    pe->i.RY = rp[2];

    if( rl >= 4 )
    {
        pe->duration_msec = rp[3]<<8;
        if( rl >= 5 )
        {
            pe->duration_msec |= rp[4];
        }
    }
    else
    {
        pe->duration_msec=default_press_duration_msec;
    }

    // Set up the up strokes.
    // Since this is a full button press, we now need
    // to release the buttons.

    pe=NULL;
    CMDQueueAdd(&pe);
    if( ! pe )
    {
        // This shouldn't happen.
        ReplyOverflow();
        return;
    }

    // It's already initialized to its default state so
    // we don't have to do anything.

    ReplySuccess();
}

void RequestPressHat(uint8_t *rp, uint8_t rl)
{
    if( rl < 2 )
    {
        ReplyError();
    }
    if( rl > 4 )
    {
        ReplyError();
    }

    uint8_t f=CMDQueueFree();
    if( f < 2 )
    {
        ReplyOverflow();
        return;
    }

    // Set up the down strokes.
    cmdqueue_element_t *pe=NULL;
    CMDQueueAdd(&pe);
    if( ! pe )
    {
        ReplyOverflow();
        return;
    }

    // Request and reply data is MSB-first AKA Network Byte Order AKA Big-Endian
    // 0       1    2          3
    // Prefix, Hat, MSec high, MSec low

    pe->i.HAT = rp[1];

    if( rl >= 3 )
    {
        pe->duration_msec = rp[2]<<8;
        if( rl >= 4 )
        {
            pe->duration_msec |= rp[3];
        }
    }
    else
    {
        pe->duration_msec=default_press_duration_msec;
    }

    // Set up the up strokes.
    // Since this is a full button press, we now need
    // to release the buttons.

    pe=NULL;
    CMDQueueAdd(&pe);
    if( ! pe )
    {
        // This shouldn't happen.
        ReplyOverflow();
        return;
    }

    // It's already initialized to its default state so
    // we don't have to do anything.

    ReplySuccess();
}

void RequestClearState(uint8_t *rp, uint8_t rl)
{
    CMDQueueClear_gpe();
    CMDQueueReset();
    ReplySuccess();
}

void RequestPauseMSec(uint8_t *rp, uint8_t rl)
{
    ReplyError();
}

void RequestReportPending(uint8_t *rp, uint8_t rl)
{
    ReplyError();
}

void ProcessRequest(uint8_t *rp, uint8_t rl)
{
    if( rl < 1 )
    {
        ReplyError();
        return;
    }

    uint8_t request_command=rp[0];

    switch(request_command)
    {
        default:
            ReplyError();
            break;
        case GBPCMD_REQ_TEST:
            ReplyAlive();
            break;
        case GBPCMD_REQ_QUERY_STATE:
            RequestQueryState(rp,rl);
            break;
        case GBPCMD_REQ_DEBUG:
            RequestDebug(rp,rl);
            break;
        case GBPCMD_REQ_GET_USB_OUT_DATA:
            RequestGetUSBOutData(rp,rl);
            break;
        case GBPCMD_REQ_SET_ALL:
            RequestSetAll(rp,rl);
            break;
        case GBPCMD_REQ_SET_BUTTONS:
            RequestSetButtons(rp,rl);
            break;
        case GBPCMD_REQ_SET_LEFT_JOY:
            RequestSetLeftJoy(rp,rl);
            break;
        case GBPCMD_REQ_SET_RIGHT_JOY:
            RequestSetRightJoy(rp,rl);
            break;
        case GBPCMD_REQ_SET_HAT:
            RequestSetHat(rp,rl);
            break;
        case GBPCMD_REQ_UNSET_ALL:
            RequestUnsetAll(rp,rl);
            break;
        case GBPCMD_REQ_SET_DOWN_MSEC:
            RequestSetDownMSec(rp,rl);
            break;
        case GBPCMD_REQ_PRESS_ALL:
            RequestPressAll(rp,rl);
            break;
        case GBPCMD_REQ_PRESS_BUTTONS:
            RequestPressButtons(rp,rl);
            break;
        case GBPCMD_REQ_MOVE_LEFT_JOY:
            RequestMoveLeftJoy(rp,rl);
            break;
        case GBPCMD_REQ_MOVE_RIGHT_JOY:
            RequestMoveRightJoy(rp,rl);
            break;
        case GBPCMD_REQ_PRESS_HAT:
            RequestPressHat(rp,rl);
            break;
        case GBPCMD_REQ_CLEAR_STATE:
            RequestClearState(rp,rl);
            break;
        case GBPCMD_REQ_PAUSE_MSEC:
            RequestPauseMSec(rp,rl);
            break;
        case GBPCMD_REQ_REPORT_PENDING:
            RequestReportPending(rp,rl);
            break;
    }
}
