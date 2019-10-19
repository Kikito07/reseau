#include "utils.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sender_list.h>
#include <time.h>

int real_address(char *address, struct sockaddr_in6 *rval)
{
  struct addrinfo hints, *result;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  int err = getaddrinfo(address, NULL, &hints, &result);
  if (err != 0)
  {
    return -1;
  }
  memcpy(rval, result->ai_addr, result->ai_addrlen);
  return 0;
}

int read_file_and_send(char *filename, int sock)
{
  int seqn = 0;
  // opening file
  if (filename == NULL)
  {
    printf("filename is NULL\n");
    return -1;
  }
  // opening file
  int fd = open(filename, O_RDWR);
  if (fd < 0)
  {
    printf("fail to open\n");
    return -1;
  }
  // retrieving size
  struct stat buf;
  fstat(fd, &buf);
  int sz = buf.st_size;
  if (sz == 0)
  {
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
  while (sz >= 512)
  {
    bytes_r = read(fd, buf_payload, 512);
    if (bytes_r == -1)
    {
      printf("read fail\n");
      return -1;
    }
    pkt_t *pkt = pkt_new();
    pkt_set_payload(pkt, buf_payload, 512);
    pkt_set_seqnum(pkt, seqn);
    pkt_set_timestamp(pkt, time(NULL));
    sz -= 512;
    size_t encoded_byte = 528;
    if (pkt_encode(pkt, pkt_encoded, &encoded_byte) != 0)
    {
      printf("encode error\n");
      close(fd);
      return -1;
    }
    ssize_t byte_sent;
    list_err = push(list, pkt);
    if (list_err == -1)
    {
      close(fd);
      return -1;
    }
    seqn = (seqn + 1) % 256;
  }
  if (sz > 0)
  {
    bytes_r = read(fd, buf_payload, sz);
    if (bytes_r == -1)
    {
      printf("read fail\n");
      return -1;
    }
    pkt_t *pkt = pkt_new();
    pkt_set_payload(pkt, buf_payload, sz);
    pkt_set_seqnum(pkt, seqn);
    pkt_set_timestamp(pkt, time(NULL));
    size_t encoded_byte = 528;
    if (pkt_encode(pkt, pkt_encoded, &encoded_byte) != 0)
    {
      printf("encode error\n");
      close(fd);
      return -1;
    }
    ssize_t byte_sent;
    list_err = push(list, pkt);
    if (list_err == -1)
    {
      close(fd);
      return -1;
    }
  }
  return 0;
}