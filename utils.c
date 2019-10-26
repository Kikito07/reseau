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
  int timeout_msecs = 2000;
  int ret;
  fds[0].fd = sock;
  fds[0].events = POLLIN;

  pkt_t *ender = pkt_new();
  int err;
  pkt_set_length(ender, 0);
  pkt_set_seqnum(ender, *seqn);
  pkt_set_window(ender, 0);
  pkt_set_payload(ender, NULL, 0);

  list_t *ender_list = init_list();
  list_add(ender_list, ender);
  err = pkt_send(ender, sock);
  if (err != 0) {
    fprintf(stderr, "send fail\n");
    return -1;
  }
  // printf("after send\n");
  // printf("inside end_connection\n");
  while (ender_list->size > 0) {
    ret = poll(fds, 1, timeout_msecs);
    if (ret == 0) {
      break;
    }
    if (ret > 0) {
      if (fds[0].revents & POLLIN) {
        pkt_receive(ender_list, sock);
        fds[0].revents = 0;
      }
    }
  }
  free_list(ender_list);
  close(sock);
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
    printf("send fail inside function\n");
    return -1;
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

  // printf("seq_mod = %d\n", seq_mod);
  // printf("first : %d\n", list->first->pkt->seqnum);
  // printf("last : %d\n", list->last->pkt->seqnum);
  // printf("size : %d\n", list->size);

  // searching for seqnum - 1
  while (runner != NULL) {
    if (runner->pkt->seqnum == seq_mod) {
      // printf("seqnum_send ack_routine %d\n", (pkt_get_seqnum(pkt) - 1) %
      // 256);
      found = true;
      // printf("index : %d\n", index);
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
    // printf("move winow\n");
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
        return -1;
      }
    }
    runner = runner->next;
  }
  return pkt_get_seqnum(pkt);
}
int pkt_receive(list_t *list, int sock) {
  // printf("in receiv\n");
  char buffer[528];
  pkt_t decoded;
  int byte_received;
  pkt_status_code err;
  byte_received = recv(sock, buffer, 528, 0);
  if (byte_received == -1) {
    return -1;
  }
  // printf("Bytes_received : %d\n", byte_received);
  // printf("before decode\n");
  err = pkt_decode(buffer, byte_received, &decoded);
  if (err != 0) {
    printf("err : %d \n", (int)err);
    return -1;
  }
  // printf("after decode\n");
  // printf("seqnum receive : %d\n", pkt_get_seqnum(&decoded));

  if (pkt_get_type(&decoded) == PTYPE_DATA) {
    return -1;
  }
  if (pkt_get_type(&decoded) == PTYPE_ACK) {
    // printf("in ack\n");
    return ack_routine(list, &decoded);
  }
  return nack_routine(list, &decoded, sock);
}

int read_file_and_send(char *filename, int sock) {

  // opening file
  if (filename == NULL) {
    printf("filename is NULL\n");
    return -1;
  }
  // opening file
  int fd = open(filename, O_RDWR);
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

  // filling list
  if (list_fill(list, fd, &sz, &seqn) != 0) {
    printf("error list_fill\n");
  }
  // printf("size : %d\n", list->size);

  // usefull for poll
  struct pollfd fds[1];
  int timeout_msecs = 3000;
  int ret;
  fds[0].fd = sock;
  fds[0].events = POLLIN;
  // window receiver
  int w_receiver = 0;

  // sending first packet
  double MB_read = 0;
  pkt_send(peek(list), sock);
  MB_read += 0.000512;

  for (;;) {
    ret = poll(fds, 1, timeout_msecs);
    if (ret > 0) {
      if (fds[0].revents & POLLIN) {
        // printf("inside poll\n");

        w_receiver = pkt_receive(list, sock);
        printf("w_receiver : %d\n", w_receiver);
        // if receiver is overloaded wait
        if (w_receiver == 0) {
          usleep(100);
          w_receiver = 1;
        }
        if (list_fill(list, fd, &sz, &seqn) == -1) {
          printf("error list_fill\n");
        }
        // printf("size : %d\n", list->size);
        // printf("list_is_empty : %d\n", list_is_empty(list));
        // printf("sz : %d\n", sz);
        // printf("list -> size %d\n", list->size);
        // printf("after list_fill\n");
        // printf("list_is_empty : %d\n", list_is_empty(list));
        // printf("sz : %d\n", sz);
        if (list_is_empty(list) == 1 && sz == 0) {
          // printf("before free\n");
          free(list);
          end_connection(sock, &seqn);
          // printf("MB_send : %f\n", MB_read);
          return 0;
        }
        list->window = min(w_receiver, list->size);
        // printf("window : %d\n", list->window);
      }
      fds[0].revents = 0;
    }
    node_t *runner = list->first;
    // printf("for loop list->window : %d\n", list->window);
    for (int i = 0; i < list->window; i++) {
      // printf("inside for loop %d !!!!!!!!\n", i);
      // printf("seqn : %d\n", runner->pkt->seqnum);
      // printf("diff time : %u\n", ((uint32_t) time(NULL) -
      // (uint32_t)pkt_get_timestamp(runner->pkt))); printf("timestamp %u\n",
      // (uint32_t)pkt_get_timestamp(runner->pkt)); printf("time(NULL) = %lu\n",
      // time(NULL));
      if (runner->ack == false &&
          (uint32_t)time(NULL) - (uint32_t)pkt_get_timestamp(runner->pkt) >
              (uint32_t)1) {
        if (pkt_send(runner->pkt, sock) != -1) {
          // printf("packet sent %d\n", runner->pkt->seqnum);
        }
        MB_read += 0.000512;
      }
      runner = runner->next;
    }
    // printf("end of resend\n");
  }
}
