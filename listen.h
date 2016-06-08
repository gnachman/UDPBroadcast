#ifndef __LISTEN_H__
#define __LISTEN_H__

#include <sys/socket.h>

typedef void ListenCallback(char *message,
                            int length,
                            struct sockaddr_storage *remoteAddress,
                            socklen_t remoteAddressSize);

// Starts a long-running server listening for UDP broadcasts. Returns nonzero
// on failure. Does not return on success. Invokes `callback` when a datagram
// is received.
int Listen(unsigned short port, ListenCallback *callback);

#endif  // _LISTEN_H__

