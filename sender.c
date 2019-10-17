#include "packet_interface.h"
#include "utils.h"
#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

char *filename = NULL;
struct sockaddr_in6 peer_addr;
int port;
int sock_sen, sock_rec;

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
  printf("port : %s\n", argv[optind]);

  // creating sockets
  sock_rec = socket(AF_INET6, SOCK_DGRAM, 0);
  if (sock_rec == -1) {
    printf("sock_rec fail\n");
    return -1;
  }
  sock_sen = socket(AF_INET6, SOCK_DGRAM, 0);
  if (sock_sen == -1) {
    printf("sock_sens\n");
    close(sock_rec);
    return -1;
  }
  // binding and connecting sockets
  int err;
  err = connect(sock_sen, (const struct sockaddr *)&peer_addr,
                sizeof(struct sockaddr_in6));
  if (err == -1) {
    printf("connect failed\n");
    close(sock_rec);
    close(sock_sen);
    return -1;
  }
  if (filename == NULL) {
    printf("no filename\n");
    return -1;
  }
  if (read_file_and_send(filename, sock_sen) == -1) {
    printf("fail to send\n");
    return -1;
  }
  printf("sent \n");
  close(sock_rec);
  close(sock_sen);
  return 0;
}
