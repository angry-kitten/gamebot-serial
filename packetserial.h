/*
Copyright 2021 by angry-kitten
Serial packet support written for gamebot-serial.
*/

#ifndef _PACKETSERIAL_H
#define _PACKETSERIAL_H

#define SERIAL_RING_SIZE    (32)
typedef struct serial_ring_t {
    uint8_t head; // incremented as bytes added
    uint8_t tail; // incremented as bytes removed
    uint8_t count; // number of bytes of data present
    uint8_t ring[SERIAL_RING_SIZE];
} serial_ring_t;

extern serial_ring_t sri;
extern serial_ring_t sro;

// packetserial.c
uint8_t SerialRingUsed(serial_ring_t *rp);
uint8_t SerialRingFree(serial_ring_t *rp);
void SerialRingAdd(serial_ring_t *rp, uint8_t b);
void SerialRingAddString(serial_ring_t *rp, const char *s);
uint8_t SerialRingPop(serial_ring_t *rp);
void SerialOutRingTask(void);
uint8_t SerialRingPeek(serial_ring_t *rp, uint8_t offset);
void SerialPacketTask(void);
void ProcessPacket(void);
uint8_t LowerEightCRC32(uint8_t *data, uint8_t data_len);
void ReplyPacket(uint8_t *d, uint8_t dlen);

// request.c
void ReplyError(void);
void ProcessRequest(uint8_t *rp, uint8_t rl);

#endif /* _PACKETSERIAL_H */


