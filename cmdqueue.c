/*
Copyright 2021 by angry-kitten
Serial packet support written for gamebot-serial.
*/

#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Drivers/Peripheral/Serial.h>
#include <LUFA/Platform/Platform.h>

#include "gamebotserial.h"
#include "cmdqueue.h"

cmdqueue_t cmdq={0,0,0}; // ring not initialized

void CMDQueueReset(void)
{
    cmdq.count=0;
    cmdq.head=0;
    cmdq.tail=0;
}

uint8_t CMDQueueUsed(void)
{
    return cmdq.count;
}

uint8_t CMDQueueFree(void)
{
    uint8_t f=(CMDQUEUE_SIZE-1)-(cmdq.count);
    return f;
}

void SetElementDefaultState(cmdqueue_element_t *pe)
{
    memset(pe,0,sizeof(cmdqueue_element_t));

    pe->i.Button=0;
    pe->i.HAT=HAT_CENTER;
    pe->i.LX=STICK_CENTER;
    pe->i.LY=STICK_CENTER;
    pe->i.RX=STICK_CENTER;
    pe->i.RY=STICK_CENTER;
}

// Add an item to the queue by returning a pointer to the next
// available element. It's a kinda reversed. Add it first and
// fill it in second.
void CMDQueueAdd(cmdqueue_element_t **ppe)
{
    *ppe=NULL;

    uint8_t f=CMDQueueFree();
    if( 0 == f )
    {
        return;
    }
    
    *ppe=&(cmdq.ring[cmdq.head]);
    cmdq.head=(cmdq.head+1)%CMDQUEUE_SIZE;
    cmdq.count++;

    SetElementDefaultState(*ppe);
}

// Return a pointer to the popped element.
void CMDQueuePop(cmdqueue_element_t **ppe)
{
    *ppe=NULL;

    uint8_t u=CMDQueueUsed();
    if( 0 == u )
    {
        return;
    }
    *ppe=&(cmdq.ring[cmdq.tail]);
    cmdq.tail=(cmdq.tail+1)%CMDQUEUE_SIZE;
    cmdq.count--;
}

