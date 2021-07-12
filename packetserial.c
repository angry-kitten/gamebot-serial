/*
Copyright 2021 by angry-kitten
Serial packet support written for gamebot-serial.
*/

#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Drivers/Peripheral/Serial.h>
#include <LUFA/Platform/Platform.h>

#include "gamebotserial.h"
#include "packetserial.h"
#include "crc.h"

serial_ring_t sri={0,0,{0}};
serial_ring_t sro={0,0,{0}};

/*
Packet format
byte 0 == packet start, SP_START
byte 1 == length 0 to 15 bytes to follow duplicated in upper and lower 4 bits
byte n == data
byte -2 = checksum
byte -1 = packet end, SP_END
*/
#define SP_START            'P'
#define SP_END              'E'
#define SP_MAX_DATA_SIZE    (15)
#define SP_MIN_SIZE         (4)
#define SP_MAX_SIZE         (19)
#define SP_LEN_INVERT       (0xf0)

uint8_t packet_length=0;
uint8_t packet[SP_MAX_SIZE];
uint8_t reply_packet_length=0;
uint8_t reply_packet[SP_MAX_SIZE];

uint8_t SerialRingUsed(serial_ring_t *rp)
{
    uint8_t used=0;

    if( rp->tail <= rp->head )
    {
        used=rp->head-rp->tail;
    }
    else
    {
        used=SERIAL_RING_SIZE-(rp->tail-rp->head);
    }

    return used;
}

uint8_t SerialRingFree(serial_ring_t *rp)
{
    uint8_t f=(SERIAL_RING_SIZE-1)-SerialRingUsed(rp);
    return f;
}

void SerialRingAdd(serial_ring_t *rp, uint8_t b)
{
    uint8_t f=SerialRingFree(rp);
    if( 0 == f )
    {
        return; // Throw b away.
    }
    
    if( 1 == f )
    {
        // indicate buffer full and force an error
        b='X';
    }

    rp->ring[rp->head]=b;
    rp->head=(rp->head+1)%SERIAL_RING_SIZE;
}

void SerialRingAddString(serial_ring_t *rp, const char *s)
{
    if( ! s )
    {
        return;
    }

    const char *walk=s;
    while( *walk )
    {
        SerialRingAdd(rp,*walk);
        walk++;
    }
}

uint8_t SerialRingPop(serial_ring_t *rp)
{
    uint8_t u=SerialRingUsed(rp);
    if( 0 == u )
    {
        return 0;
    }
    uint8_t returnme=rp->ring[rp->tail];
    rp->tail=(rp->tail+1)%SERIAL_RING_SIZE;
    return returnme;
}

void SerialOutRingTask(void)
{
    uint8_t u=SerialRingUsed(&sro);
    if( 0 == u )
    {
        return;
    }

    if( ! Serial_IsSendReady() )
    {
        return;
    }

    uint8_t b=SerialRingPop(&sro);

    Serial_SendByte(b);
}

uint8_t SerialRingPeek(serial_ring_t *rp, uint8_t offset)
{
    uint8_t u=SerialRingUsed(&sri);
    if( offset >= u )
    {
        // can't peek past the end of data
        return 0;
    }

    uint8_t index=(rp->tail+offset)%SERIAL_RING_SIZE;
    uint8_t returnme=rp->ring[index];
    return returnme;
}

void SerialPacketTask(void)
{
    while(true)
    {
        // See how much is in the input ring.
        uint8_t u=SerialRingUsed(&sri);
        if( u < SP_MIN_SIZE )
        {
            // Not enough data for a packet.
            return;
        }

        // Look at the first byte without removing it from the ring.
        uint8_t fb=SerialRingPeek(&sri,0);
        if( SP_START != fb )
        {
            // Throw the start byte away.
            (void)SerialRingPop(&sri);
            continue;
        }

        // Look at the encoded length.
        uint8_t lengths=SerialRingPeek(&sri,1);
        lengths^=SP_LEN_INVERT;
        uint8_t l1=lengths&0x0f;
        uint8_t l2=(lengths&0xf0)>>4;

        if( l1 != l2 )
        {
            // Throw the start byte away.
            (void)SerialRingPop(&sri);
            break; // continue;
        }

        // start, lengths, data, checksum, end
        uint8_t computed_length=1+1+l1+1+1;

        if( u < computed_length )
        {
            // Not enough data has arrived yet.
            break;
        }

        // Verify the end byte;
        uint8_t e=SerialRingPeek(&sri,computed_length-1);
        if( SP_END != e )
        {
            // Throw the start byte away.
            (void)SerialRingPop(&sri);
            break; // continue;
        }


        // It looks like a packet.
        packet_length=computed_length;
        uint8_t i;
        for(i=0;i<computed_length;i++)
        {
            packet[i]=SerialRingPop(&sri);
        }

        ProcessPacket();
    }
}

void ReplyPacket(uint8_t *d, uint8_t dlen)
{
    if( dlen > SP_MAX_DATA_SIZE )
    {
        return;
    }

    uint8_t l=dlen|(dlen<<4);
    l^=SP_LEN_INVERT;

    SerialRingAdd(&sro,SP_START);
    SerialRingAdd(&sro,l);
    int i=0;
    for(i=0;i<dlen;i++)
    {
        SerialRingAdd(&sro,d[i]);
    }

    uint8_t crc32ish=LowerEightCRC32(d,dlen);
    SerialRingAdd(&sro,crc32ish);

    SerialRingAdd(&sro,SP_END);
}

void ReplyPacketError(void)
{
    uint8_t rdata[1];

    rdata[0]=GBPCMD_REP_ERROR;

    ReplyPacket(rdata,1);
}

void ReplyPacketAlive(void)
{
    uint8_t rdata[1];

    rdata[0]=GBPCMD_REP_ALIVE;

    ReplyPacket(rdata,1);
}

void ProcessRequest(uint8_t *rp, uint8_t rl)
{
    if( rl < 1 )
    {
        ReplyPacketError();
        return;
    }

    uint8_t request_command=rp[0];

    switch(request_command)
    {
        default:
            ReplyPacketError();
            break;
        case GBPCMD_REQ_TEST:
            ReplyPacketAlive();
            break;
    }
}

// Return the lower 8 bits of a crc32.
uint8_t LowerEightCRC32(uint8_t *data, uint8_t data_len)
{
    uint32_t crc1=F_CRC_CalculaCheckSum(data,data_len);
    return (crc1&0xff);
}

void ProcessPacket(void)
{
    uint8_t pl=packet_length;
    packet_length=0;

    if( pl < 4 )
    {
        ReplyPacketError();
        return;
    }

    if( SP_START != packet[0] )
    {
        ReplyPacketError();
        return;
    }

    if( SP_END != packet[pl-1] )
    {
        ReplyPacketError();
        return;
    }

    uint8_t *data=&(packet[2]);
    uint8_t data_len=pl-4;

    uint8_t packet_crc32ish=packet[pl-2];
    uint8_t crc32ish=LowerEightCRC32(data,data_len);

    if( packet_crc32ish != crc32ish )
    {
        ReplyPacketError();
        return;
    }

    ProcessRequest(data,data_len);
}
