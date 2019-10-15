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
  /*---------------------OK-----------------------*/
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
  /*----------------------Ok--------------------------*/

  pkt_t *pkt = pkt_new();
  char *msg = "\nHello world\n";
  pkt_set_payload(pkt, msg, strlen(msg) + 1);
  pkt_set_seqnum(pkt, 0);
  pkt_set_timestamp(pkt, 0);
  pkt_set_tr(pkt, 0);
  pkt_set_type(pkt, PTYPE_DATA);
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

  int err2 = 0;
  err2 = send(sock_sen, buffer_send, length, 0);
  if (err2 == -1) {
    return -1;
  }
  char buffer_rec[528];
  err2 = recv(sock_rec, buffer_rec, length, 0);
  if (err2 == -1) {
    return -1;
  }
  pkt_t *decoded = pkt_new();
  if (pkt_decode(buffer_rec, length, decoded) != 0) {
    return -1;
  }
  printf("payload : %s", pkt_get_payload(decoded)); 
}