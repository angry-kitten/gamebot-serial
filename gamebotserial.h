/*
Copyright 2021 by angry-kitten
General header for gamebot-serial.
*/

#ifndef _GAMEBOTSERIAL_H
#define _GAMEBOTSERIAL_H

#define GB_MAJOR_VERSION    1
#define GB_MINOR_VERSION    1

#define GBSTR2(x)    #x
#define GBSTR(x)    GBSTR2(x)

#define GB_VERSION_STRING   GBSTR(GB_MAJOR_VERSION) "." GBSTR(GB_MINOR_VERSION)

// These are the prefixes for the commands in the data
// part of the packets.
#define GBPCMD_REQ_TEST                 'T'
#define GBPCMD_REQ_QUERY_STATE          'Q'
#define GBPCMD_REQ_DEBUG                'D'
#define GBPCMD_REQ_GET_USB_OUT_DATA     'O' // capital O
#define GBPCMD_REQ_SET_ALL              'S'
#define GBPCMD_REQ_SET_BUTTONS          'B'
#define GBPCMD_REQ_SET_LEFT_JOY         'L'
#define GBPCMD_REQ_SET_RIGHT_JOY        'R'
#define GBPCMD_REQ_SET_HAT              'H'
#define GBPCMD_REQ_UNSET_ALL            'U'
#define GBPCMD_REQ_SET_DOWN_MSEC        'M'
#define GBPCMD_REQ_PRESS_ALL            's'
#define GBPCMD_REQ_PRESS_BUTTONS        'b'
#define GBPCMD_REQ_PRESS_LEFT_JOY       'l'
#define GBPCMD_REQ_PRESS_RIGHT_JOY      'r'
#define GBPCMD_REQ_PRESS_HAT            'h'
#define GBPCMD_REQ_CLEAR_STATE          'C'
#define GBPCMD_REQ_PAUSE_MSEC           'P'
#define GBPCMD_REQ_REPORT_PENDING       'p'

#define GBPCMD_REP_ALIVE            'A'
// Define these error numbers as prefix characters so we can have single
// byte responses instead of a prefix plus a number.
#define GBPCMD_REP_SUCCESS          '0'  // AKA no error
#define GBPCMD_REP_ERROR            '1'
#define GBPCMD_REP_OVERFLOW         '2'
#define GBPCMD_REP_3                '3'
#define GBPCMD_REP_4                '4'
#define GBPCMD_REP_5                '5'
#define GBPCMD_REP_6                '6'
#define GBPCMD_REP_7                '7'
#define GBPCMD_REP_8                '8'
#define GBPCMD_REP_9                '9'

#define GBPCMD_REQ_QUERY_STATE_REPLY_SIZE           (14)
// Flags for the first status byte of the GBPCMD_REQ_QUERY_STATE reply.
#define GB_FLAGS_CONFIGURED                         (0x01)

#endif /* _GAMEBOTSERIAL_H */


