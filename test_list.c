#include "sender_list.h"

int main() {
  list_t *my_list = init_list();
  pkt_t *pkt1 = pkt_new();
  pkt_set_payload(pkt1, "1", 2);
  pkt_t *pkt2 = pkt_new();
  pkt_set_payload(pkt2, "2", 2);
  pkt_t *pkt3 = pkt_new();
  pkt_set_payload(pkt3, "3", 2);
  pkt_t *pkt4 = pkt_new();
  pkt_set_payload(pkt4, "4", 2);
  pkt_t *pkt5 = pkt_new();
  pkt_set_payload(pkt5, "5", 2);
  push(my_list, pkt1);
  push(my_list, pkt2);
  push(my_list, pkt3);
  push(my_list, pkt4);
  push(my_list, pkt5);
  pkt_t *printer;
  printer = peek(my_list);
  printf("%s\n", pkt_get_payload(printer));
  delete (my_list);
  printer = peek(my_list);
  printf("%s\n", pkt_get_payload(printer));
  delete (my_list);
  printer = peek(my_list);
  printf("%s\n", pkt_get_payload(printer));
  delete (my_list);
  printer = peek(my_list);
  printf("%s\n", pkt_get_payload(printer));
  delete (my_list);
  printer = peek(my_list);
  printf("%s\n", pkt_get_payload(printer));
  delete (my_list);
  if (my_list->first == NULL) {
    printf("ok\n");
  }
  return 0;
}