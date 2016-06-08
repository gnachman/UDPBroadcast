#include "listen.h"

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

static void DatagramReceived(char *message,
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
}

int main(int argc, char *argv[]) {
  return Listen(1912, DatagramReceived);
}
