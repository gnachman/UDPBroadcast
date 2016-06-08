#ifndef __LISTEN_H__
#define __LISTEN_H__

#include <sys/socket.h>

// Return a nonzero value to stop listening.
typedef int ListenCallback(char *message,
                           int length,
                           struct sockaddr_storage *remoteAddress,
                           socklen_t remoteAddressSize);

// Starts a long-running server listening for UDP broadcasts. Returns nonzero
// on failure. Returns zero on success (when the callback tells it to exit).
// Invokes `callback` when a datagram is received.
int Listen(unsigned short port, ListenCallback *callback);

#endif  // _LISTEN_H__

