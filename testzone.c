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

int main() {
  pkt_t *pkt = pkt_new();
  memset(pkt, 0,sizeof(pkt_t));
  char payload[10] = "Hello";
  pkt_set_payload(pkt, payload, strlen(payload) + 1);
  pkt_set_seqnum(pkt, 0);
  pkt_set_timestamp(pkt, 0);
  pkt_set_window(pkt, 0);

  size_t len = 512;
  char encoded[512];
  char decoded[512];
  
  if(pkt_encode(pkt, encoded, &len) != 0){
      printf("error\n");
  }

  pkt_t *pkt_decoded = pkt_new();
  pkt_decode(encoded, len, pkt_decoded);
  printf("pl : %s\n", pkt_get_payload(pkt_decoded));
  return 0;
}