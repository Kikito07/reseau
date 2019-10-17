#include "packet_interface.h"
#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

char *filename;
struct sockaddr_in6 peer_addr;
int port;
int sock_rec, sock_sen;

int main(int argc, char *argv[]) {

  char c;

  while ((c = getopt(argc, argv, "f:")) != -1) {
    switch (c) {
    case 'f':
      filename = (char *)malloc(strlen(optarg) + 1);
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
  // retieving adress
  if (optind == argc) {
    printf("you must specify an adress\n");
    return -1;
  }
  memset(&peer_addr, 0, sizeof(peer_addr));
  peer_addr.sin6_family = AF_INET6;
  if (inet_pton(AF_INET6, argv[optind], &peer_addr.sin6_addr) != 1) {
    printf("adresse isn't valid\n");
    return -1;
  }
  printf("adress : %s\n", argv[optind]);
  optind++;

  // retriving port
  if (optind == argc) {
    printf("you must specify a port\n");
    return -1;
  }
  peer_addr.sin6_port = htons(atoi(argv[optind]));
  printf("port : %s\n", argv[optind]);

  // packet creation
  pkt_t *pkt = pkt_new();
  char *msg = "Hello";
  pkt_set_payload(pkt, msg, strlen(msg) + 1);
  pkt_set_seqnum(pkt, 1);
  pkt_set_timestamp(pkt, 1);
  int pkt_size = 0;
  pkt_size += predict_header_length(pkt);
  pkt_size += 4;
  if (pkt_get_tr(pkt) == 0) {
    pkt_size += pkt_get_length(pkt);
    pkt_size += 4;
  }
  char buffer_send[528];
  size_t length = 528;
  if (pkt_encode(pkt, buffer_send, &length) != 0) {
    printf("error encode ; \n");
    return -1;
  }
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
  int err_send = 0;
  //char *msg2 = "kiko\n";
  //err_send = send(sock_sen, msg, strlen(msg2) + 1, 0);
  err_send = send(sock_sen, buffer_send, length, 0);
  if (err_send == -1) {
    printf("send error :\n");
    close(sock_rec);
    close(sock_sen);
    return -1;
  }
  printf("sent \n");
  close(sock_rec);
  close(sock_sen);
  return 0;
}
