/*
Copyright 2021 by angry-kitten
Command queue gamebot-serial.
*/

#ifndef _CMDQUEUE_H
#define _CMDQUEUE_H

#include "Joystick.h"

#define CMDQUEUE_SIZE           (8)
#define DEFAULT_BUTTON_PRESS_DURATION       (55)  // msec

typedef struct cmdqueue_element_t {
    USB_JoystickReport_Input_t i; // input to the host
    uint16_t duration_msec;
} cmdqueue_element_t;

typedef struct cmdqueue_t {
    uint8_t head; // incremented as items added
    uint8_t tail; // incremented as items removed
    uint8_t count; // number of items present
    cmdqueue_element_t ring[CMDQUEUE_SIZE];
} cmdqueue_t;

extern cmdqueue_t cmdq;

// cmdqueue.c
void CMDQueueReset(void);
uint8_t CMDQueueFree(void);
void CMDQueueAdd(cmdqueue_element_t **ppe);
void CMDQueuePop(cmdqueue_element_t **ppe);

#endif /* _CMDQUEUE_H */


