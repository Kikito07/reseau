#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stddef.h>
#include <string.h>
#include "packet_interface.h"
#include "sender_list.h"

int real_address(char *address, struct sockaddr_in6 *rval);
int read_file_and_send(char *filename, int sock);
int nack_routine(list_t *list, pkt_t *pkt, int sock);
int pkt_receive(list_t *list, int sock);
int ack_routine(list_t *list, pkt_t *pkt);
int pkt_send(pkt_t *pkt, int sock);
int end_connection(int sock, int *seqn);