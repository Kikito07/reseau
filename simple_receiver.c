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

  int sock_rec;
  sock_rec = socket(AF_INET6, SOCK_DGRAM, 0);
  if (sock_rec == -1) {
    return -1;
  }
  /*---------------------OK-----------------------*/
  struct sockaddr_in6 my_addr;
  memset(&my_addr, 0, sizeof(my_addr));
  my_addr.sin6_family = AF_INET6;
  my_addr.sin6_port = htons(55555);
  inet_pton(AF_INET6, "::1", &my_addr.sin6_addr);
  int err;
  err = bind(sock_rec, (const struct sockaddr *)&my_addr,
             sizeof(struct sockaddr_in6));
  if (err == -1) {
    return -1;
  }
  /*----------------------Ok--------------------------*/

  int err2 = 0;
  char buffer_rec[21];
  err2 = recv(sock_rec, buffer_rec, 528, 0);
  if (err2 == -1) {
    return -1;
  }
  printf("bytes read : %d\n", err2); 
  pkt_t *decoded = pkt_new();
  if (pkt_decode(buffer_rec, err2, decoded) != 0) {
    printf("decode error\n");
    return -1;
  }
  printf("payload : %s\n", pkt_get_payload(decoded)); 
  close(sock_rec);
  return 0;
}