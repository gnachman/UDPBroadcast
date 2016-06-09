#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <stdio.h>
#include <arpa/inet.h>

// Return a nonzero value to stop listening.
typedef int ListenCallback(char *message,
                           int length,
                           struct sockaddr_storage *remoteAddress,
                           socklen_t remoteAddressSize,
                           void *usersData);

// Create a socket that is allowed to use the broadcast interface.
// Pass 1 in wantLoopback if you want to receive messages sent to the broadcast
// address on localhost.
int GetBroadcastSocket(unsigned char wantLoopback);

// Send a message to the broadcast address and close the socket.
int BroadcastToSocket(int sock, char *messge, size_t length, unsigned short broadcastPort);

// Change whether the socket listens on the loopback interface.
int SetSocketWantsLoopback(int sock, unsigned char loopback);

// Bind a port to a socket. Messages sent to the port will get queued and can
// be read later with ReceiveFromSocket().
int BindSocketToPort(int sock, unsigned short port);

// Returns the port the socket is bound to.
unsigned short GetSocketPort(int sock);

// Tries to receive a single message on a scoket opened with GetListenSocket().
// Returns nonzero if listening should stop, either because of an error or
// because the callback returned a nonzero value.
int ReceiveFromSocket(int sock, double timeout, ListenCallback *callback, void *userData);

// Sends a message to a socket
int SendToSocket(int sock,
                 char *message,
                 size_t length,
                 struct sockaddr_in *remoteAddress);

// Send a UDP message to the specified address. May not be a broadcast address.
int Send(char *message,
         size_t length,
         struct sockaddr_in *remoteAddress);

// Send a UDP broadcast locally containing "message". The `length` should not
// exceed 65507. Closes the socket.
int Broadcast(char *message,
              size_t length,
              unsigned short broadcastPort,
              unsigned short sourcePort,
              unsigned char wantLoopback);


// Starts a long-running server listening for UDP broadcasts. Returns nonzero
// on failure. Returns zero on success (when the callback tells it to exit).
// Invokes `callback` when a datagram is received.
int RunListenServer(unsigned short port, ListenCallback *callback);

#endif  // _SOCKET_H__

