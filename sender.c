#include "packet_interface.h"
#include "utils.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

char *filename = NULL;
struct sockaddr_in6 peer_addr;
int port;
int sock_sen;

int main(int argc, char *argv[]) {

  char c;
  while ((c = getopt(argc, argv, "f:")) != -1) {
    switch (c) {
    case 'f':
      filename = (char *)malloc(strlen(optarg) + 1);
      strcpy(filename, optarg);
      if (filename == NULL) {
        printf("malloc fail\n");
        return -1;
      }
      break;

    case '?':
      printf("unknown option : %s\n", optarg);
      break;
    }
  }
  // retieving address
  if (optind == argc) {
    printf("you must specify an adress\n");
    return -1;
  }
  memset(&peer_addr, 0, sizeof(peer_addr));
  if (real_address(argv[optind], &peer_addr) == -1) {
    printf("failed to find address\n");
    return -1;
  }
  optind++;

  // retriving port
  if (optind == argc) {
    printf("you must specify a port\n");
    return -1;
  }
  peer_addr.sin6_port = htons(atoi(argv[optind]));
  // creating socket
  sock_sen = socket(AF_INET6, SOCK_DGRAM, 0);
  if (sock_sen == -1) {
    printf("sock_sens\n");
    return -1;
  }
  // binding and connecting sockets
  int err;
  err = connect(sock_sen, (const struct sockaddr *)&peer_addr,
                sizeof(struct sockaddr_in6));
  if (err == -1) {
    printf("connect failed\n");
    close(sock_sen);
    return -1;
  }
  if (filename == NULL) {

    // usefull for poll
    struct pollfd fds[1];
    int ret;
    fds[0].fd = fileno(stdin);
    fds[0].events = POLLIN;
    ret = poll(fds, 1, -1);
    for (;;) {
      if (ret > 0) {
        if (fds[0].revents & POLLIN) {
          if (read_file_and_send(0, sock_sen) == -1) {
            printf("fail to read stdin\n");
          }
          break;
        }
      }
    }

  } else {
    int fd = open(filename, O_RDWR);
    if (fd < 0) {
      printf("fail to open\n");
      return -1;
    }
    if (read_file_and_send(fd, sock_sen) == -1) {
      printf("fail to send\n");
      return -1;
    }
  }
  close(sock_sen);
  free(filename);
  return 0;
}
