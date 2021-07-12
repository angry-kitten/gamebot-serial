/*
Copyright 2021 by angry-kitten
General header for gamebot-serial.
*/

#ifndef _GAMEBOTSERIAL_H
#define _GAMEBOTSERIAL_H

#define GB_MAJOR_VERSION    1
#define GB_MINOR_VERSION    0

#define GBSTR2(x)    #x
#define GBSTR(x)    GBSTR2(x)

#define GB_VERSION_STRING   GBSTR(GB_MAJOR_VERSION) "." GBSTR(GB_MINOR_VERSION)

#endif /* _GAMEBOTSERIAL_H */


