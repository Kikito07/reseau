#include "packet_interface.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int connect2(const struct sockaddr *dest_addr, socklen_t addrlen) {
  int sock = socket(AF_INET6, SOCK_DGRAM, 0);
  if (sock == -1) {
    return -1;
  }

  int err = connect(sock, dest_addr, addrlen);
  if (err == -1) {
    return -1;
    return sock;
  }
  return 0;
}

int send2(pkt_t *pkt, int sock) {
  char buf[528];
  size_t pkt_size = 0;
  pkt_size += predict_header_length(pkt);
  pkt_size += 4;
  if (pkt_get_tr(pkt) == 0) {
    pkt_size += pkt_get_length(pkt);
    pkt_size += 4;
  }
  size_t l = 1024;
  if (pkt_encode(pkt, buf, &l) != 0) {
    return -1;
  }
  ssize_t sent = send(sock, buf, pkt_size, 0);
  if (sent == -1) {
    return -1;
  }
  return 0;
}

int main(int argc, char *argv[]) {

  int sock_rec, sock_sen;
  sock_rec = socket(AF_INET6, SOCK_DGRAM, 0);
  if (sock_rec == -1) {
    return -1;
  }
  sock_sen = socket(AF_INET6, SOCK_DGRAM, 0);
  if (sock_sen == -1) {
    return -1;
  }
  struct sockaddr_in6 peer_addr;
  memset(&peer_addr, 0, sizeof(peer_addr));
  peer_addr.sin6_family = AF_INET6;
  peer_addr.sin6_port = htons(55555);
  inet_pton(AF_INET6, "::1", &peer_addr.sin6_addr);
  int err;
  err = bind(sock_rec, (const struct sockaddr *)&peer_addr,
             sizeof(struct sockaddr_in6));
  if (err == -1) {
    return -1;
  }
  err = connect(sock_sen, (const struct sockaddr *)&peer_addr,
                sizeof(struct sockaddr_in6));
  if (err == -1) {
    return -1;
  }

  pkt_t *pkt = pkt_new();
  char *msg = "Hello";
  pkt_set_payload(pkt, msg, strlen(msg) + 1);
  pkt_set_seqnum(pkt, 0);
  pkt_set_timestamp(pkt, 0);
  pkt_set_tr(pkt, 0);
  pkt_set_type(pkt, PTYPE_DATA);
  char received[528];
  int pkt_size = 0;
  pkt_size += predict_header_length(pkt);
  pkt_size += 4;
  if (pkt_get_tr(pkt) == 0) {
    pkt_size += pkt_get_length(pkt);
    pkt_size += 4;
  }
  send2(pkt, sock_sen);
  printf("coucou\n");
  recv(sock_rec, received, pkt_size, 0);
  
  
  printf("coucou2\n");

  pkt_t *decoded_pkt = pkt_new();
  char decoded[528];
  if (pkt_decode(decoded, pkt_size, decoded_pkt) != 0) {
    printf("ca couille\n");
    return -1;

  }
  printf("coucou3\n");
  printf("payload : %s", pkt_get_payload(decoded_pkt));
}