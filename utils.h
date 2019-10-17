#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stddef.h>
#include <string.h>
#include "packet_interface.h"

int real_address(char *address, struct sockaddr_in6 *rval);
int read_file_and_send(char *filename, int sock);