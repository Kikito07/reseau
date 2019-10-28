#include "sender_list.h"
#include "packet_interface.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// init a list
list_t *init_list() {
  list_t *list = malloc(sizeof(list_t));
  list->first = NULL;
  list->last = NULL;
  list->size = 0;
  list->window = 1;
  list->marker = false;
  list->r_timer = 1000000;
  return list;
}

// free a list
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
// add an element at the end of the list
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

// delete the first element of the list
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
// peek the first element of the list
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

// peek the last element of the list
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
// set list->window
int set_window(list_t *list, int nbm_window) {
  if (list == NULL) {
    return -1;
  }
  list->window = nbm_window;
  return 0;
}

// move the sender window if possible
int list_move_window(list_t *list) {
  if (list == NULL) {
    return -1;
  }
  while (list->first != NULL && list->first->ack == true) {
    delete (list);
  }
  return 0;
}

// return -1 on failure 1 if list is empty 0 otherwise
int list_is_empty(list_t *list) {
  if (list == NULL) {
    return -1;
  }
  if (list->size > 0) {
    return 0;
  }
  return 1;
}

// debugging function
void print_list(list_t *list) {
  node_t *runner = list->first;
  while (runner != NULL) {
    printf("seq : %d\n", runner->pkt->seqnum);
  }
}

/*read from fd and fill list using seqnum seqn*/
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
  while (list->size < 31) {
    bytes_r = read(fd, buf_payload, 512);

    if (bytes_r == -1) {
      fprintf(stderr, "read fail\n");
      return -1;
    }
    if (bytes_r == 0) {
      list->marker = true;
      return 0;
    }
    printf("bytes_read %d\n", bytes_r);
    pkt_t *pkt = pkt_new();
    pkt_set_payload(pkt, buf_payload, (uint16_t)bytes_r);
    // printf("pkt->payload %d\n", pkt_get_length(pkt));
    pkt_set_seqnum(pkt, *seqn);
    pkt_set_window(pkt, 0);
    list_err = list_add(list, pkt);
    if (list_err == -1) {
      fprintf(stderr, "list_add fail\n");
      close(fd);
      return -1;
    }
    *seqn = (*seqn + 1) % 256;
  }
  return 0;
}