#ifndef __UTIL_H
#define __UTIL_H
#include <signal.h>
#include "qn_room.h"

#define REJOIN_ROOM_SIGNAL __SIGRTMIN+10

extern int get_room_token(const char* app_id, const char* room_id, const char* user_id, char* room_token);

extern void enum_av_device();

extern void rejoin_room();

#endif // __UTIL_H