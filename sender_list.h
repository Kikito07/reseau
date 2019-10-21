#include "packet_interface.h"
#include "stdbool.h"

typedef struct Node // node structure
{
  struct Node *next;
  pkt_t *pkt;
  bool ack;

} node_t;

typedef struct List // list structure
{
  node_t *first; // head of list
  int size;      // number of element in list
  int window;
} list_t;

list_t *init_list();
pkt_t *peek(list_t *list);
int push(list_t *list, pkt_t *pkt);
int delete(list_t *list);
