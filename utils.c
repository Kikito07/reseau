#include "utils.h"
#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int min(int x, int y) {
  if (x <= y) {
    return x;
  }
  return y;
}

int real_address(char *address, struct sockaddr_in6 *rval) {
  struct addrinfo hints, *result;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  int err = getaddrinfo(address, NULL, &hints, &result);

  if (err != 0) {
    return -1;
  }
  memcpy(rval, result->ai_addr, result->ai_addrlen);
  freeaddrinfo(result);
  return 0;
}

int end_connection(int sock, int *seqn) {
  // usefull for poll
  struct pollfd fds[1];
  int timeout_msecs = 0;
  int ret;
  fds[0].fd = sock;
  fds[0].events = POLLIN;

  pkt_t *ender = pkt_new();
  int err;

  pkt_set_seqnum(ender, *seqn);
  pkt_set_window(ender, 0);
  pkt_set_payload(ender, NULL, 0);

  list_t *ender_list = init_list();
  list_add(ender_list, ender);
  err = pkt_send(ender, sock);
  if (err != 0) {
    fprintf(stderr, "send fail\n");
  }
  // printf("after send\n");
  // printf("inside end_connection\n");
  for (;;) {
    ret = poll(fds, 1, timeout_msecs);
    if (ret == 0) {
      break;
    }
    if (ret > 0) {
      if (fds[0].revents & POLLIN) {
        pkt_receive(ender_list, sock);
        fds[0].revents = 0;
        if (ender_list->size == 0) {
          printf("size is 0\n");
          break;
        }
      }
    }
    pkt_send(ender, sock);
    printf("send end connection\n");
    usleep(50);
  }
  free_list(ender_list);
  close(sock);
  printf("conection ended\n");
  return 0;
}

int pkt_send(pkt_t *pkt, int sock) {
  char buffer[528];
  size_t encoded = 528;
  pkt_set_timestamp(pkt, time(NULL));
  if (pkt_encode(pkt, buffer, &encoded) != 0) {
    printf("encode fail\n");
    return -1;
  }
  if (send(sock, buffer, encoded, 0) == -1) {
    printf("send fail function\n");
  }
  printf("seqnum_send %d\n", pkt->seqnum);
  return 0;
}

//-1 on error pkt window on succes
int ack_routine(list_t *list, pkt_t *pkt) {
  // settings window according to packet
  // printf("ack_receive : %d\n", pkt->seqnum);
  int index = 0;
  node_t *runner = list->first;
  bool found = false;
  runner = list->first;

  int seq_mod;
  if (pkt_get_seqnum(pkt) == 0) {
    seq_mod = 255;
  } else {
    seq_mod = (pkt_get_seqnum(pkt) - 1) % 256;
  }
  // searching for seqnum - 1
  while (runner != NULL) {
    if (runner->pkt->seqnum == seq_mod) {
      found = true;
      break;
    }
    runner = runner->next;
    index++;
  }

  if (found == false) {
    return pkt->window;
  }
  runner = list->first;
  for (int i = 0; i <= index; i++) {
    if (runner->ack == false) {
      printf("acked : %d\n", runner->pkt->seqnum);
      runner->ack = true;
    }
    runner = runner->next;
  }
  // move window
  if (list_is_empty(list) == 0) {
    list_move_window(list);
  }
  return pkt->window;
}

//-1 on error pkt window on succes
int nack_routine(list_t *list, pkt_t *pkt, int sock) {
  // settings window according to packet
  printf("nack_routine\n");
  node_t *runner = list->first;
  for (int i = 0; i < list->window; i++) {
    if (runner->pkt->seqnum == pkt->seqnum) {
      if (pkt_send(runner->pkt, sock) == -1) {
        printf("send fail\n");
      }
    }
    runner = runner->next;
  }
  return 1;
}
int pkt_receive(list_t *list, int sock) {

  printf("pkt received\n");
  char buffer[528];
  pkt_t decoded;
  int byte_received;
  pkt_status_code err;
  byte_received = recv(sock, buffer, 528, 0);
  if (byte_received == -1) {
    return -1;
  }
  err = pkt_decode(buffer, byte_received, &decoded);
  if (err != 0) {
    printf("err decode : %d \n", (int)err);
    return list->window;
  }

  if (pkt_get_type(&decoded) == PTYPE_DATA) {
    printf("wrong type\n");
    return -1;
  }
  if (pkt_get_type(&decoded) == PTYPE_ACK) {

    return ack_routine(list, &decoded);
  }
  return nack_routine(list, &decoded, sock);
}

int read_file_and_send(int fd, int sock) {

  // initializing list and seqnum
  list_t *list = init_list();
  int seqn = 0;

  // filling list
  if (list_fill(list, fd, &seqn) != 0) {
    fprintf(stderr, "error list_fill\n");
  }

  // usefull for poll
  struct pollfd fds[1];
  int timeout_msecs = 0;
  int ret;
  fds[0].fd = sock;
  fds[0].events = POLLIN;
  // window receiver
  int w_receiver = 0;

  // sending first packet
  pkt_send(peek(list), sock);
  for (;;) {
    printf("inside for loop\n");
    printf("before poll\n");
    ret = poll(fds, 1, timeout_msecs);
    printf("afte poll\n");
    
    if (ret > 0) {
      if (fds[0].revents & POLLIN) {
        printf("inside pollin\n");
        w_receiver = pkt_receive(list, sock);
        // if receiver is overloaded wait
        if (w_receiver == 0) {
          printf("window received = 0\n");
          usleep(100);
          w_receiver = 1;
        }
        printf("before list_fill\n");
        if (list_fill(list, fd, &seqn) == -1) {
          fprintf(stderr, "error list_fill\n");
        }
        printf("after list_fill\n");
        if (list_is_empty(list) == 1 && list->marker == true) {
          free(list);
          end_connection(sock, &seqn);
          return 0;
        }
        list->window = min(w_receiver, list->size);
      }
      fds[0].revents = 0;
    }
    node_t *runner = list->first;
    for (int i = 0; i < list->window; i++) {

      if (runner->ack == false &&
          (uint32_t)time(NULL) - (uint32_t)pkt_get_timestamp(runner->pkt) >
              (uint32_t)1) {
                printf("seqn = %d\n", runner->pkt->seqnum);
        if (pkt_send(runner->pkt, sock) != -1) {
        }
      }
      runner = runner->next;
    }
  }
}
