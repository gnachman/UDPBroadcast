#include <stdlib.h>

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

int Broadcast(char *message,
              size_t length,
              unsigned short broadcastPort,
              unsigned char wantLoopback) {
  int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock < 0){
    fprintf(stderr, "socket() failed: %s", strerror(errno));
    return 1;
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
    return 1;
  }

  status = setsockopt(sock,
                      IPPROTO_IP,
                      IP_MULTICAST_LOOP,
                      (void *)&wantLoopback,
                      sizeof(wantLoopback));
  if (status < 0){
    fprintf(stderr, "setsockopt(multicast) failed: %s", strerror(errno));
    return 1;
  }

  /* Construct local address structure */
  char *broadcastIP = "255.255.255.255";
  struct sockaddr_in broadcastAddr;
  memset(&broadcastAddr, 0, sizeof(broadcastAddr));
  broadcastAddr.sin_family = AF_INET;
  broadcastAddr.sin_addr.s_addr = inet_addr(broadcastIP);
  broadcastAddr.sin_port = htons(broadcastPort);

  status = sendto(sock,
                  message,
                  length,
                  0,
                  (struct sockaddr *)&broadcastAddr,
                  sizeof(broadcastAddr));
  if (status != length) {
    fprintf(stderr, "sendto() failed: %s", strerror(errno));
    return 1;
  }

  return 0;
}

