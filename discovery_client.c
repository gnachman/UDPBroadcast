#include "libdiscovery.h"
#include "log.h"
#include "socket.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: discovery_client payload");
    return 1;
  }

  size_t length = 0;
  char *message = SendDiscoveryRequest(argv[1], &length);
  if (message) {
    printf("%.*s\n", (int)length, message);
  }
  return message == NULL;
}
