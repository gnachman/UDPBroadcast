#include "socket.h"

#include "log.h"
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

int SetSocketWantsLoopback(int sock, unsigned char wantLoopback) {
  int status = setsockopt(sock,
                          IPPROTO_IP,
                          IP_MULTICAST_LOOP,
                          (void *)&wantLoopback,
                          sizeof(wantLoopback));
  if (status < 0){
    fprintf(stderr, "setsockopt(multicast) failed: %s", strerror(errno));
    return -1;
  }

  return 0;
}

int GetBroadcastSocket(unsigned char wantLoopback) {
  int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock < 0){
    fprintf(stderr, "socket() failed: %s", strerror(errno));
    return -1;
  }

  int broadcastPermission = 1;
  int status;
  status = setsockopt(sock,
                      SOL_SOCKET,
                      SO_BROADCAST,
                      (void *)&broadcastPermission,
                      sizeof(broadcastPermission));
  if (status < 0){
    fprintf(stderr, "setsockopt(broadcast) failed: %s", strerror(errno));
    close(sock);
    return -1;
  }

  status = SetSocketWantsLoopback(sock, wantLoopback);
  if (status < 0) {
    close(sock);
    return -1;
  }

  return sock;
}

int BindSocketToPort(int sock, unsigned short port) {
  struct sockaddr_in localSocketAddress;
  memset(&localSocketAddress, 0, sizeof(localSocketAddress));
  localSocketAddress.sin_family = AF_INET;
  localSocketAddress.sin_port = htons(port);
  localSocketAddress.sin_addr.s_addr = INADDR_ANY;

  int status = bind(sock,
                    (struct sockaddr *)&localSocketAddress,
                    sizeof(struct sockaddr));
  if (status < 0){
    fprintf(stderr, "bind() failed: %s\n", strerror(errno));
    close(sock);
    return -1;
  }

  return 0;
}


int BroadcastToSocket(int sock,
                      char *message,
                      size_t length,
                      unsigned short broadcastPort) {
  char *broadcastIP = "255.255.255.255";
  struct sockaddr_in broadcastAddr;
  memset(&broadcastAddr, 0, sizeof(broadcastAddr));
  broadcastAddr.sin_family = AF_INET;
  broadcastAddr.sin_addr.s_addr = inet_addr(broadcastIP);
  broadcastAddr.sin_port = htons(broadcastPort);
  return SendToSocket(sock, message, length, &broadcastAddr);
}

int Send(char *message,
         size_t length,
         struct sockaddr_in *remoteAddress) {
  int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock < 0){
    fprintf(stderr, "socket() failed: %s", strerror(errno));
    return 1;
  }

  int status = SendToSocket(sock, message, length, remoteAddress);
  close(sock);

  return status;
}

int SendToSocket(int sock,
                 char *message,
                 size_t length,
                 struct sockaddr_in *remoteAddress) {
  int status;
  Log("Send datagram to %s:%d\n",
      inet_ntoa(remoteAddress->sin_addr),
      (int)ntohs(remoteAddress->sin_port));
  status = sendto(sock,
                  message,
                  length,
                  0,
                  (struct sockaddr *)remoteAddress,
                  sizeof(*remoteAddress));
  Log("Returned from sendto\n");
  if (status != length) {
    fprintf(stderr, "sendto() failed in socket.c: %s (%d)\n", strerror(errno), errno);
    return 1;
  }

  return 0;
}

int Broadcast(char *message,
              size_t length,
              unsigned short broadcastPort,
              unsigned short sourcePort,
              unsigned char wantLoopback) {
  int sock = GetBroadcastSocket(wantLoopback);

  int status;
  if (sourcePort > 0) {
    struct sockaddr_in sourceAddress;
    memset(&sourceAddress, 0, sizeof(sourceAddress));
    sourceAddress.sin_family = AF_INET;
    sourceAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    sourceAddress.sin_port = htons(sourcePort);

    status = bind(sock, (struct sockaddr *)&sourceAddress, sizeof(sourceAddress));
    if (status < 0) {
      fprintf(stderr, "bind() of source port %d failed: %s\n", (int)sourcePort, strerror(errno));
    }
  }

  return BroadcastToSocket(sock, message, length, broadcastPort);
}

unsigned short GetSocketPort(int sock) {
  socklen_t length;
  struct sockaddr_in address = {};
  length = sizeof(address);
  getsockname(sock, (struct sockaddr *)&address, &length);
  return ntohs(address.sin_port);
}

static double CurrentTime() {
  struct timeval now;
  gettimeofday(&now, NULL);
  return now.tv_sec + ((double)now.tv_usec) / 1000000.0;
}

// Returns 0 on success, nonzero if a timeout or error occurred.
int WaitForReadOnSocket(int fd, double timeout) {
  struct timeval tv;
  tv.tv_sec = floor(timeout);
  tv.tv_usec = fmod(timeout, 1.0) * 1000000;
  fd_set rfds;
  int status;
  do {
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);
    status = select(fd + 1, &rfds, NULL, NULL, &tv);
  } while (status < 0 && (errno == EINTR || errno == EAGAIN));
  return status <= 0;
}

int ReceiveFromSocket(int sock, double timeout, ListenCallback *callback, void *userData) {
  double deadline = CurrentTime() + timeout;

  while (1) {
    char buffer[65508];
    struct sockaddr_storage remoteSocketAddress;
    socklen_t socketAddressSize = sizeof(remoteSocketAddress);
    if (!isinf(timeout)) {
      if (WaitForReadOnSocket(sock, deadline - CurrentTime())) {
        return 1;
      }
    }
    int status = recvfrom(sock,
                          buffer,
                          sizeof(buffer),
                          0,
                          (struct sockaddr *)&remoteSocketAddress,
                          &socketAddressSize);
    if (status < 0) {
      if (errno == EAGAIN || errno == EINTR) {
        continue;
      }
      fprintf(stderr, "recvfrom() failed: %s", strerror(errno));
      return 1;
    } else {
      return callback(buffer, status, &remoteSocketAddress, socketAddressSize, userData);
    }
  }
}

int RunListenServer(unsigned short port, ListenCallback *callback) {
  int sock = GetBroadcastSocket(1);
  if (sock < 0) {
    fprintf(stderr, "Failed to create socket\n");
    return 1;
  }
  if (BindSocketToPort(sock, port)) {
    fprintf(stderr, "Failed to bind to port\n");
    return 1;
  }

  while (1) {
    if (ReceiveFromSocket(sock, INFINITY, callback, NULL)) {
      fprintf(stderr, "Finished\n");
      break;
    }
  }

  close(sock);

  return 0;
}

