#include "utils.h"
#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// calculate de minimum between two int
int min(int x, int y) {
  if (x <= y) {
    return x;
  }
  return y;
}

// fonction that finds addresses
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

/*sends a data packet of length 0 and wait for its ack
@sock : socket connected to peer
@seqn : seqnum of last packet send
@return : return 0 on sucess */
int end_connection(int sock, int *seqn) {
  // variable used for poll
  struct pollfd fds[1];
  int timeout_msecs = 4000;
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
  for (;;) {
    ret = poll(fds, 1, timeout_msecs);
    if (ret == 0) {
      break;
    }
    if (ret > 0) {
      if (fds[0].revents & POLLIN) {
        pkt_receive(ender_list, sock);
        if (ender_list->size == 0) {
          break;
        }
      }
    }
    fds[0].revents = 0;
    pkt_send(ender, sock);
    usleep(50);
  }
  free_list(ender_list);
  close(sock);
  return 0;
}
/*sends a packet through a socket*
@pkt : packet to send
@sock : socket connected to peer
@return : -1 if encoding or sending fail 0 on success*/

int pkt_send(pkt_t *pkt, int sock) {
  char buffer[528];
  size_t encoded = 528;
  // usefull for retransmission
  pkt_set_timestamp(pkt, time(NULL));
  if (pkt_encode(pkt, buffer, &encoded) != 0) {
    fprintf(stderr, "encode fail\n");
    return -1;
  }
  if (send(sock, buffer, encoded, 0) == -1) {
    fprintf(stderr, "send fail\n");
  }
  return 0;
}

/*routine used when an Ack is received. The method goes through the list and
 * acknowledges packets then shifts the sender window if possible.
 * @list : a linked list contaning the sender window
 * @pkt : the packet received
 * @return :  return pkt->window*/
int ack_routine(list_t *list, pkt_t *pkt) {
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
  // if seqnum receveied is not in window return
  if (found == false) {
    return pkt->window;
  }
  runner = list->first;
  for (int i = 0; i <= index; i++) {
    if (runner->ack == false) {
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

/*routine used when a packet of type Nack is received. If the packet is in the
window send it back else return
@list : the list used to store the window
@pkt : the packet received
@sock : a socket connected to the peer
@return : return 0*/
int nack_routine(list_t *list, pkt_t *pkt, int sock) {
  // settings window according to packet
  node_t *runner = list->first;
  for (int i = 0; i < list->window; i++) {
    if (runner->pkt->seqnum == pkt->seqnum) {
      if (pkt_send(runner->pkt, sock) == -1) {
        fprintf(stderr, "send fail\n");
      }
    }
    runner = runner->next;
  }
  return 0;
}

/*fonction called when something can be read from the socket. It then calls the
 * nack or ack routine depending on the packet received
 * @list : list used to stock the window
 * @sock : socked connected to peer
 * @return : return the window of the received packet on success and
 * list->window on failure */
int pkt_receive(list_t *list, int sock) {
  char buffer[528];
  pkt_t decoded;
  int byte_received;
  pkt_status_code err;
  byte_received = recv(sock, buffer, 528, 0);
  if (byte_received == -1) {
    fprintf(stderr, "read fail\n");
    return list->window;
  }
  err = pkt_decode(buffer, byte_received, &decoded);
  if (err != 0) {
    fprintf(stderr, "error in decode\n");
    return list->window;
  }

  if (pkt_get_type(&decoded) == PTYPE_DATA) {
    fprintf(stderr, "wrong type received\n");
    return list->window;
  }
  if (pkt_get_type(&decoded) == PTYPE_ACK) {

    return ack_routine(list, &decoded);
  }
  return nack_routine(list, &decoded, sock);
}

/*reads from file descriptor and sends the content through a socket
@fd : a file descriptor
@sock : a socket connected to peer
@return : 0 on success, -1 on failure
*/
int read_file_and_send(int fd, int sock) {

  // initializing list and seqnum
  list_t *list = init_list();
  int seqn = 0;

  // filling list
  if (list_fill(list, fd, &seqn) != 0) {
    fprintf(stderr, "error list_fill\n");
    return -1;
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
    ret = poll(fds, 1, timeout_msecs);
    if (ret > 0) {
      if (fds[0].revents & POLLIN) {
        w_receiver = pkt_receive(list, sock);
        // if receiver is overloaded wait
        if (w_receiver == 0) {
          usleep(100);
          w_receiver = 1;
        }
        if (list_fill(list, fd, &seqn) == -1) {
          fprintf(stderr, "error list_fill\n");
          return -1;
        }
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
        if (pkt_send(runner->pkt, sock) != -1) {
        }
      }
      runner = runner->next;
    }
  }
}
