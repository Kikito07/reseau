#include "sender_list.h"
#include "utils.h"
#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int main() {

  int fd = open("lorem.txt", O_RDWR);
  if (fd < 0) {
    printf("fail to open\n");
    return -1;
  }
  // retrieving size
  struct stat buf;
  fstat(fd, &buf);
  int sz = buf.st_size;
  if (sz == 0) {
    close(fd);
    return -1;
  }
  // initializing list and seqnum
  list_t *list = init_list();
  int seqn = 0;
  list_fill(list, fd, &sz, &seqn);
  printf("list size in test %d\n", list->size);
  while(list->size > 0){
    printf("in while\n");
    printf("payload : %s\n", peek(list)->payload);
    delete(list);
  }
  printf("after\n");
  return 0;

  /*pkt_t *pkt1 = pkt_new();
  pkt_t *pkt2 = pkt_new();
  pkt_t *pkt3 = pkt_new();
  pkt_t *pkt4 = pkt_new();

  pkt_set_payload(pkt1, "1", 2);
  pkt_set_payload(pkt2, "2", 2);
  pkt_set_payload(pkt3, "3", 2);
  pkt_set_payload(pkt4, "4", 2);
  list_add(list, pkt1);
  list_add(list, pkt2);
  list_add(list, pkt3);
  list_add(list, pkt4);
  while(list->size > 0){
    printf("in while\n");
    printf("payload : %s\n", peek(list)->payload);
    delete(list);
  }
  printf("after\n");
  return 0;*/
}