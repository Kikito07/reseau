#include "sender_list.h"
#include "packet_interface.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

list_t *init_list() {
  list_t *list = malloc(sizeof(list_t));
  list->first = NULL;
  list->last = NULL;
  list->size = 0;
  list->window = 1;
  list->marker = false;
  return list;
}

int free_list(list_t *list) {
  if (list == NULL) {
    return 0;
  }
  while (list->size > 0) {
    delete (list);
  }
  free(list);
  return 0;
}

int list_add(list_t *list, pkt_t *pkt) {

  node_t *new = malloc(sizeof(node_t));
  new->ack = false;
  new->next = NULL;
  new->pkt = pkt;
  new->ack = false;
  if (list->size == 0) {
    list->first = new;
    list->last = new;
    list->size++;
    return 0;
  }
  list->last->next = new;
  list->last = new;
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

pkt_t *peek_last(list_t *list) {
  if (list == NULL) {
    return NULL;
  }
  if (list->first == NULL) {
    return NULL;
  } else {
    return list->last->pkt;
  }
}

int set_window(list_t *list, int nbm_window) {
  if (list == NULL) {
    return -1;
  }
  list->window = nbm_window;
  return 0;
}

int list_move_window(list_t *list) {
  if (list == NULL) {
    return -1;
  }
  while (list->first != NULL && list->first->ack == true) {
    // printf("inside list_move_window\n");
    delete (list);
    // printf("after delete\n");
  }
  // printf("out of while loop\n");
  if (list->first == NULL) {
    // printf("list->first == NULL\n");
    return -2;
  }
  // printf("before return list_move_window\n");
  return 0;
}

int list_is_empty(list_t *list) {
  if (list == NULL) {
    return -1;
  }
  if (list->size > 0) {
    return 0;
  }
  return 1;
}

void print_list(list_t *list) {
  node_t *runner = list->first;
  while (runner != NULL) {
    printf("seq : %d\n", runner->pkt->seqnum);
  }
}

int list_fill(list_t *list, int fd, int *seqn) {

  if (list == NULL) {
    fprintf(stderr, "list is NULL\n");
    return -1;
  }
  if (list->marker == true) {
    return 0;
  }
  char buf_payload[512];
  int bytes_r;
  int list_err;

  // reading file and filling list
  bytes_r = read(fd, buf_payload, 512);
 
  if (bytes_r == -1) {
    printf("read fail\n");
    return -1;
  }
   if (bytes_r == 0) {
    list->marker = true;
    printf("bytes_r 0\n");
    return 0;
  }
  pkt_t *pkt = pkt_new();
  pkt_set_payload(pkt, buf_payload, bytes_r);
  pkt_set_seqnum(pkt, *seqn);
  pkt_set_timestamp(pkt, 0);
  pkt_set_window(pkt, 0);
  list_err = list_add(list, pkt);
  // printf("list size %d\n", list->size);
  if (list_err == -1) {
    printf("list_add fail\n");
    close(fd);
    return -1;
  }
  *seqn = (*seqn + 1) % 256;
  return 0;
}