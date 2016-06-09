#include "socket.h"

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

static int DatagramReceived(char *message,
                            int length,
                            struct sockaddr_storage *remoteAddress,
                            socklen_t remoteAddressSize) {
  char formattedHostName[NI_MAXHOST];
  char formattedPortNumber[NI_MAXSERV];

  int status = getnameinfo((struct sockaddr *)remoteAddress,
                           remoteAddressSize,
                           formattedHostName,
                           sizeof(formattedHostName),
                           formattedPortNumber,
                           sizeof(formattedPortNumber),
                           NI_NUMERICHOST | NI_NUMERICSERV);
  printf("Received message\n");
  if (status == 0) {
    printf("Sender: %s:%s\n", formattedHostName, formattedPortNumber);
  } else {
    printf("Sender unknown: %s\n", gai_strerror(status));
  }

  printf("Content: %.*s\n", length, message);
  printf("\n");

  return 1;
}

int main(int argc, char *argv[]) {
  return RunListenServer(1912, DatagramReceived);
}
