#include "sender_list.h"
#include <stdlib.h>

list_t *init_list() {
  list_t *list = malloc(sizeof(list_t));
  list->first = NULL;
  list->last = NULL;
  list->size = 0;
  list->window = 1;
  return list;
}

int add(list_t *list, pkt_t *pkt) {

  node_t *new = malloc(sizeof(node_t));
  new->next = NULL;
  new->pkt = pkt;
  new->ack = false;
  if (list->size == 0) {
    list->first = new;
    list-> last = new;
    list->size++;
    return 0;
  }
    list->last->next = new;
    list->last = list->last->next;
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
    list->last = NULL;
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

pkt_t *peek_last(list_t *list){
    if (list == NULL) {
    return NULL;
  }
  if (list->first == NULL) {
    return NULL;
  } else {
    return list->last->pkt;
  }
}

int set_window(list_t *list, int nbm_window){
  if(list == NULL){
    return -1;
  }
  list->window = nbm_window;
  return 0;
}

int list_move_window(list_t *list){
  if(list == NULL){
    return -1;
  }
  while(list->first->ack == true){
    delete(list);
  }
  return 0;
}