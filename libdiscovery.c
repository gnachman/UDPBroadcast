#include "libdiscovery.h"

#include "log.h"
#include "socket.h"
#include <stdio.h>
#include <string.h>

typedef struct {
  char *message;
  size_t length;
} LibDiscoveryUserData;

static int HandleResponse(char *message,
                          int length,
                          struct sockaddr_storage *remoteAddress,
                          socklen_t remoteAddressSize,
                          void *userData) {
  LibDiscoveryUserData *myUserData = (LibDiscoveryUserData *)userData;

  if (length > 0) {
    myUserData->message = malloc(length);
    memmove(myUserData->message, message, length);
  } else {
    myUserData->message = NULL;
  }

  myUserData->length = length;

  return 0;
}

char *SendDiscoveryRequest(char *payload, size_t *length) {
  Log("Get listen socket...\n");
  int sock = GetBroadcastSocket(0);
  if (sock < 0) {
    Log("Could not listen on socket.\n");
    return NULL;
  }

  int status = BindSocketToPort(sock, 0);
  if (status) {
    Log("Failed to bind\n");
    return NULL;
  }

  Log("Set wants loopback...\n");
  status = SetSocketWantsLoopback(sock, 1);
  if (status) {
    return NULL;
  }

  Log("Get the port...\n");
  int port = GetSocketPort(sock);
  Log("Port is %d\n", port);

  Log("Send a broadcast...\n");
  status = BroadcastToSocket(sock, payload, strlen(payload), 1912);
  if (status < 0) {
    Log("Failed to broadcast\n");
    return NULL;
  }

  Log("Receive...\n");
  LibDiscoveryUserData userData;
  status = ReceiveFromSocket(sock, 1.0, HandleResponse, &userData);
  Log("status: %d", status);
  if (status) {
    return NULL;
  } else {
    *length = userData.length;
    return userData.message ?: "";
  }
}
