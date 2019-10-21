#include "utils.h"
#include <fcntl.h>
#include <poll.h>
#include <sender_list.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

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
  return 0;
}

int pkt_send(pkt_t *pkt, int sock) {
  char buffer[528];
  int encoded = 528;
  pkt_set_timestamp(pkt, time(NULL));
  if (pkt_encode(pkt, buffer, &encoded) != 0) {
    return -1;
  }
  if (send(sock, buffer, encoded, 0) == -1) {
    return -1;
  }
  return 0;
}

int ack_routine(list_t *list, pkt_t *pkt) {
  // settings window according to packet
  int index = 0;
  node_t *runner = list->first;
  bool found = false;
  runner = list->first;
  // searching for seqnum - 1
  for (index = 0; index < list->window; index++) {
    if (pkt_get_seqnum(runner->pkt) == (pkt_get_seqnum(pkt) - 1) % 256) {
      found = true;
      break;
    }
  }
  if (found == false) {
    return 0;
  }
  for (int i = 0; i <= index; i++) {
    runner = list->first;
    if (runner->ack == false) {
      runner->ack = true;
      // if first element of window is acknowledged we can move the window
    }
  }
  list_move_window(list);//move window
  list->window = pkt->window;
  return 0;
}

int nack_routine(list_t *list, pkt_t *pkt, int sock) {
  // settings window according to packet
  list->window = pkt->window;
  for (int i = 0; i < list->window; i++) {
    node_t *runner = list->first;
    if (runner->pkt->seqnum == pkt->seqnum) {
      if (pkt_send(runner->pkt, sock) == -1) {
        return -1;
      }
    }
  }
  return 0;
}

int pkt_receive(list_t *list, int sock) {
  char buffer[528];
  pkt_t decoded;
  int byte_received;
  byte_recv(sock, buffer, 512, 0);
  if (byte_received == -1) {
    return -1;
  }
  if (pkt_decode(buffer, byte_received, &decoded) != 0) {
    return -1;
  }
  if (pkt_get_type(&decoded) == PTYPE_DATA) {
    return -1;
  }
  if (pkt_get_type(&decoded) == PTYPE_ACK) {
    return ack_routine(list, &decoded);
  }
  return nck_routine(list, &decoded, sock);
}

int read_file_and_send(char *filename, int sock) {
  int seqn = 0;
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
  // initializing variables

  char pkt_encoded[528];
  char buf_payload[512];
  int bytes_r;
  int list_err;
  list_t *list = list_init();

  // reading file and sending it
  while (sz >= 512) {
    bytes_r = read(fd, buf_payload, 512);
    if (bytes_r == -1) {
      printf("read fail\n");
      return -1;
    }
    pkt_t *pkt = pkt_new();
    pkt_set_payload(pkt, buf_payload, 512);
    pkt_set_seqnum(pkt, seqn);
    sz -= 512;
    size_t encoded_byte = 528;
    if (pkt_encode(pkt, pkt_encoded, &encoded_byte) != 0) {
      printf("encode error\n");
      close(fd);
      return -1;
    }
    ssize_t byte_sent;
    list_err = push(list, pkt);
    if (list_err == -1) {
      close(fd);
      return -1;
    }
    seqn = (seqn + 1) % 256;
  }
  if (sz > 0) {
    bytes_r = read(fd, buf_payload, sz);
    if (bytes_r == -1) {
      printf("read fail\n");
      return -1;
    }
    pkt_t *pkt = pkt_new();
    pkt_set_payload(pkt, buf_payload, sz);
    pkt_set_seqnum(pkt, seqn);
    size_t encoded_byte = 528;
    if (pkt_encode(pkt, pkt_encoded, &encoded_byte) != 0) {
      printf("encode error\n");
      close(fd);
      return -1;
    }
    ssize_t byte_sent;
    list_err = push(list, pkt);
    if (list_err == -1) {
      close(fd);
      return -1;
    }
  }

  struct pollfd fds[1];
  int timeout_msecs = 3000;
  int ret;
  int i;
  fds[0].fd = sock;
  fds[0].events = POLLIN;

  // sending first packet

  pkt_send(peek(list), sock);

  for (;;) {
    ret = poll(fds, 1, timeout_msecs);
    if (ret > 0) {
      pkt_receive(list, sock);
      fds[0].revents = 0;
    }
    for (int i = 0; i < list->window; i++) {
      node_t *runner = peek(list);
      if (runner->ack == false) {
        pkt_send(runner, sock);
      }
    }
    for (int i = 0; i < list->window; i++) {
      node_t *runner = peek(list);
      if (runner->ack == false &&
          (time(NULL) - pkt_get_timestamp(runner->next) > 2)) {
        pkt_send(runner, sock);
      }
    }

    return 0;
  }
