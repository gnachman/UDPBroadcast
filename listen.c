#include "listen.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

int Listen(unsigned short port, ListenCallback *callback) {
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock < 0) {
    fprintf(stderr, "socket() failed: %s", strerror(errno));
    return 1;
  }

  int broadcast=1;

  int status;
  status = setsockopt(sock,
                      SOL_SOCKET,
                      SO_BROADCAST,
                      &broadcast,
                      sizeof(broadcast));
  if (status < 0){
    fprintf(stderr, "setsockopt(broadcast) failed: %s", strerror(errno));
    return 1;
  }

  struct sockaddr_in localSocketAddress;
  memset(&localSocketAddress, 0, sizeof(localSocketAddress));
  localSocketAddress.sin_family = AF_INET;
  localSocketAddress.sin_port = htons(port);
  localSocketAddress.sin_addr.s_addr = INADDR_ANY;

  status = bind(sock,
                (struct sockaddr *)&localSocketAddress,
                sizeof(struct sockaddr));
  if (status < 0){
    fprintf(stderr, "bind() failed: %s", strerror(errno));
    return 1;
  }

  while (1) {
    char buffer[65508];
    struct sockaddr_storage remoteSocketAddress;
    socklen_t socketAddressSize = sizeof(remoteSocketAddress);
    status = recvfrom(sock,
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
      if (callback(buffer, status, &remoteSocketAddress, socketAddressSize)) {
        break;
      }
    }
  }

  return 0;
}

