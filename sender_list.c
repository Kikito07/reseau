#include "sender_list.h"
#include <stdlib.h>

list_t *init_list() {
  list_t *list = malloc(sizeof(list_t));
  list->first = NULL;
  list->size = 0;
  return list;
}

int push(list_t *list, pkt_t *pkt) {

  node_t *new = malloc(sizeof(node_t));
  new->next = NULL;
  new->pkt = pkt;
  if (list->size == 0) {
    list->first = new;
    list->size++;
    return 0;
  }

  if (list->size == 1) {
    list->first->next = new;
    list->size++;
    return 0;
  }
  node_t *curr = list->first;
  node_t *curr_next = curr->next;
  while (curr_next != NULL) {
    curr = curr_next;
    curr_next = curr_next->next;
  }
  curr->next = new;
  list->size++;
  return 0;
}

int delete (list_t *list) {
  if (list == NULL) {
    return -1;
  }
  if (list->first == NULL) {
    return -1;
  }
  node_t *deleted = list->first;
  list->first = list->first->next;
  free(deleted->pkt);
  free(deleted);
  list->size--;
  if (list->size == 0) {
    list->first = NULL;
  }
  return 0;
}

pkt_t *peek(list_t *list) {
  if (list == NULL) {
    return NULL;
  }
  if (list->first == NULL) {
    return NULL;
  } else {
    return list->first->pkt;
  }
}