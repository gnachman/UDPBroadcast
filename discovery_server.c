#include "log.h"
#include "socket.h"
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>

static char *gCommand;
static char *gExpectedPayload;

typedef enum {
  DiscoveryServerStatusOK,
  DiscoveryServerStatusBadRequestPayload,
  DiscoveryServerStatusBadProtocol,
  DiscoveryServerStatusSendError
} DiscoveryServerStatus;

static char *FormattedStatus(DiscoveryServerStatus status) {
  switch (status) {
    case DiscoveryServerStatusOK:
      return "OK";

    case DiscoveryServerStatusBadRequestPayload:
      return "Bad Request Payload";

    case DiscoveryServerStatusBadProtocol:
      return "Bad Protocol";

    case DiscoveryServerStatusSendError:
      return "Send Failed";
  }
}

static void RunCommand(char *command, char *buffer, size_t bufferSize) {
  FILE *commandPipe = popen(command, "r");
  size_t bytesRead = 0;
  while (!feof(commandPipe) &&
         !ferror(commandPipe) &&
         bytesRead + 1 < bufferSize) {
    const size_t spaceAvailable = bufferSize - bytesRead - 1;
    Log("Reading with %d bytes available from fd %d\n", (int)spaceAvailable, fileno(commandPipe));
    size_t result = fread(buffer + bytesRead,
                          1,
                          spaceAvailable,
                          commandPipe);
    Log("Read returned %d\n", (int)result);
    bytesRead += result;
    if (result < spaceAvailable) {
      break;
    }
  }
  if (ferror(commandPipe)) {
      Log("Command %s failed: %s", command, strerror(errno));
  }
  pclose(commandPipe);
  buffer[bytesRead] = '\0';
}

static char *FormattedHostName(struct sockaddr_storage *remoteAddress,
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
  char *result = NULL;
  if (status == 0) {
    asprintf(&result, "%s:%s", formattedHostName, formattedPortNumber);
  } else {
    asprintf(&result, "unknown (%s)", gai_strerror(status));
  }
  return result;
}

static void LogRequest(char *hostname, DiscoveryServerStatus status) {
  time_t rawtime = time(NULL);
  struct tm *timeinfo = localtime(&rawtime);
  char formattedTime[26];
  asctime_r(timeinfo, formattedTime);

  // Replace first newline character with null.
  char *temp = formattedTime;
  strsep(&temp, "\r\n");

  printf("%s: %s (%s)\n", formattedTime, hostname, FormattedStatus(status));
  fflush(stdout);
}

static DiscoveryServerStatus HandleRequest(char *message,
                                           int length,
                                           struct sockaddr_storage *remoteAddress,
                                           socklen_t remoteAddressSize) {
  if (length != strlen(gExpectedPayload) ||
      memcmp(message, gExpectedPayload, strlen(gExpectedPayload))) {
    Log("Datagram with unexpected payload:\n%.*s", length, message);
    return DiscoveryServerStatusBadRequestPayload;
  }

  if (remoteAddress->ss_family != AF_INET) {
    Log("Datagram received from non-ip4 address\n");
    return DiscoveryServerStatusBadProtocol;
  }

  Log("Running %s\n", gCommand);
  char buffer[60000];
  RunCommand(gCommand, buffer, sizeof(buffer));

  Log("Sending \"%s\"\n", buffer);
  int status = Send(buffer, strlen(buffer), (struct sockaddr_in *)remoteAddress);
  if (status) {
    Log("Failed to respond\n");
    return DiscoveryServerStatusSendError;
  }

  return DiscoveryServerStatusOK;
}

static int DatagramReceived(char *message,
                            int length,
                            struct sockaddr_storage *remoteAddress,
                            socklen_t remoteAddressSize,
                            void *userData) {
  DiscoveryServerStatus status = HandleRequest(message,
                                               length,
                                               remoteAddress,
                                               remoteAddressSize);
  char *hostname = FormattedHostName(remoteAddress, remoteAddressSize);
  LogRequest(hostname, status);
  free(hostname);
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: discovery_server expected-payload command\n");
    return 1;
  }
  gExpectedPayload = argv[1];
  gCommand = argv[2];
  return RunListenServer(1912, DatagramReceived);
}


