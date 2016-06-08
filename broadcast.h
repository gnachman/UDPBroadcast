#ifndef __BROADCAST_H__
#define __BROADCAST_H__

#include <stdio.h>

// Send a UDP broadcast locally containing "message". The `length` should not exceed 65507.
int Broadcast(char *message,
              size_t length,
              unsigned short broadcastPort,
              unsigned char wantLoopback);

#endif  // _BROADCAST_H__
