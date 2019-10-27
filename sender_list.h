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
  node_t *last;  // tail of list
  int size;      // number of element in list
  int window;    // window of the reseau
  bool marker;
} list_t;

list_t *init_list();
pkt_t *peek(list_t *list);
pkt_t *peek_last(list_t *list);
int list_add(list_t *list, pkt_t *pkt);
int delete (list_t *list);
int list_move_window(list_t *list);
int list_is_empty(list_t *list);
int list_fill(list_t *list, int fd, int *seqn);
void print_list(list_t *list);
int free_list(list_t *list);
