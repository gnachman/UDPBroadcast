#include "broadcast.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: test_client message\n");
    return 1;
  }

  return Broadcast(argv[1], strlen(argv[1]), 1912, 1);
}
