#include "packet_interface.h"
#include <arpa/inet.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <zlib.h>

/* Extra #includes */
/* Your code will be inserted here */
pkt_t *pkt_new() {
  pkt_t *pkt = malloc(sizeof(pkt_t));
  return pkt;
}

void pkt_del(pkt_t *pkt) { free(pkt); }

pkt_status_code pkt_decode(const char *data, const size_t len, pkt_t *pkt) {
  size_t index = 0;

  // decoding first byte
  uint8_t header1 = *data;

  pkt_set_type(pkt, header1 >> 6);

  // decoding tr
  header1 = header1 << 2;
  header1 = header1 >> 7;
  pkt_set_tr(pkt, header1);

  // decoding window
  header1 = *data;
  header1 = header1 << 3;
  header1 = header1 >> 3;
  pkt_set_window(pkt, header1);

  index++;

  // decoding length
  uint16_t pl_length;
  ssize_t decoded = varuint_decode((uint8_t *)(data + index), 2, &pl_length);
  pkt_set_length(pkt, pl_length);
  index += decoded;

  // decoding segnum
  pkt_set_seqnum(pkt, *(uint8_t *)(data + index));
  index++;

  // decoding timestamp
  uint32_t timestamp = *(uint32_t *)(data + index);
  pkt_set_timestamp(pkt, timestamp);
  index += 4;

  // decoding crc1
  uint32_t *crc1 = (uint32_t *)(data + index);
  *crc1 = ntohl(*crc1);
  pkt_set_crc1(pkt, (*crc1));
  index += 4;

  // calculating crc1
  uint32_t crc1_cal = 0;
  int h_length = predict_header_length(pkt);
  // tr at 1
  if (pkt_get_tr(pkt) == 1) {
    uint8_t b[h_length];
    memcpy(b, data, h_length);
    *b &= ~(1 << 5);
    crc1_cal = crc32(0, (Bytef *)(b), h_length);
  }
  // tr at 0
  else {
    crc1_cal = crc32(0, (Bytef *)(data), h_length);
  }
  // comparing calculated crc1 with received crc1
  // printf("crc1 : %u\n", pkt_get_crc1(pkt));
  // printf("crc1_cal : %u\n", crc1_cal);
  if (pkt_get_crc1(pkt) != crc1_cal) {
    return E_CRC;
  }

  // decoding payload and setting crc2
  if (pkt_get_tr(pkt) == 1) {
    return PKT_OK;
  }
  size_t payload_length = pkt_get_length(pkt);
  char payload[payload_length];
  memcpy(payload, (data + index), payload_length);
  // printf("payload : %s\n", payload);
  pkt_set_payload(pkt, payload, payload_length);
  index += payload_length;
  // printf("payload_length %u\n", payload_length);
  // decoding crc2
  uint32_t *crc2 = (uint32_t *)(data + index);
  *crc2 = ntohl(*crc2);
  // comparing the two crcs
  // printf("crc2 : %u\n", pkt_get_crc2(pkt));
  // printf("crc2_cal : %u\n", *crc2);
  if (pkt_get_crc2(pkt) != *crc2) {
    return E_CRC;
  }
  index += 4;
  if (len != index) {
    return E_UNCONSISTENT;
  }
  return PKT_OK;
}

pkt_status_code pkt_encode(const pkt_t *pkt, char *buf, size_t *len) {
  // verfying that there's enough memory in the buffer
  size_t pkt_size = 0;
  pkt_size += predict_header_length(pkt);
  pkt_size += 4;
  if (pkt_get_tr(pkt) == 0) {
    pkt_size += pkt_get_length(pkt);
    pkt_size += 4;
  }
  if ((*len) < pkt_size) {
    return E_NOMEM;
  }
  // index
  int index = 0;
  // first byte
  *buf = pkt_get_type(pkt) << 6;
  *buf |= pkt_get_tr(pkt) << 5;
  *buf |= pkt_get_window(pkt);
  index++;
  // encoding length
  int16_t length = pkt_get_length(pkt);
  ssize_t encoded = varuint_encode(length, (uint8_t *)(buf + index), 2);
  index += encoded;
  // encoding seqnum
  uint8_t sn = pkt_get_seqnum(pkt);
  *(buf + index) = sn;
  index += 1;
  // encoding timestamp
  uint32_t ts = pkt_get_timestamp(pkt);
  memcpy((buf + index), &ts, 4);
  index += 4;
  // encoding crc1
  // tr at 1
  int h_length = predict_header_length(pkt);
  if (pkt_get_tr(pkt) == 1) {
    uint8_t b[h_length];
    memcpy(b, buf, h_length);
    *b &= ~(1 << 5);
    uint32_t crc1 = crc32(0, (Bytef *)(b), h_length);
    crc1 = htonl(crc1);
    memcpy((buf + index), &crc1, 4);
    // printf("crc11 : %u\n", crc1);
  }
  // tr at 0
  else {
    uint32_t crc1 = crc32(0, (Bytef *)(buf), h_length);
    crc1 = htonl(crc1);
    memcpy((buf + index), &crc1, 4);
    // printf("crc11 : %u\n", crc1);
  }
  index += 4;
  // encoding payload
  if (pkt_get_tr(pkt) == 1) {
    return PKT_OK;
  }
  uint16_t l = pkt_get_length(pkt);
  char *pl = (char *)(pkt_get_payload(pkt));
  memcpy((buf + index), pl, l);
  index += l;
  // encoding crc2
  uint32_t crc2 = pkt_get_crc2(pkt);
  crc2 = htonl(crc2);
  memcpy(buf + index, &crc2, 4);
  index += 4;
  *len = index;
  return PKT_OK;
}

ptypes_t pkt_get_type(const pkt_t *pkt) { return (pkt->type); }

uint8_t pkt_get_tr(const pkt_t *pkt) { return pkt->TR; }

uint8_t pkt_get_window(const pkt_t *pkt) { return pkt->window; }

uint8_t pkt_get_seqnum(const pkt_t *pkt) { return pkt->seqnum; }

uint16_t pkt_get_length(const pkt_t *pkt) { return pkt->length; }

uint32_t pkt_get_timestamp(const pkt_t *pkt) { return pkt->timestamp; }

uint32_t pkt_get_crc1(const pkt_t *pkt) { return pkt->crc1; }

uint32_t pkt_get_crc2(const pkt_t *pkt) { return pkt->crc2; }

const char *pkt_get_payload(const pkt_t *pkt) { return pkt->payload; }

pkt_status_code pkt_set_type(pkt_t *pkt, const ptypes_t type) {
  // veryfing that type argument is valid
  if (type != PTYPE_DATA && type != PTYPE_ACK && type != PTYPE_NACK) {
    return E_TYPE;
  }
  pkt->type = type;
  return PKT_OK;
}

pkt_status_code pkt_set_tr(pkt_t *pkt, const uint8_t tr) {
  // veryfing that tr argument is valid
  if (tr != 0 && tr != 1) {
    return E_TR;
  }
  pkt->TR = tr;
  return PKT_OK;
}

pkt_status_code pkt_set_window(pkt_t *pkt, const uint8_t window) {
  // veryfing that window argument is valid
  if (window > 31) {
    return E_WINDOW;
  }
  pkt->window = window;
  return PKT_OK;
}

pkt_status_code pkt_set_seqnum(pkt_t *pkt, const uint8_t seqnum) {
  pkt->seqnum = seqnum;
  return PKT_OK;
}

pkt_status_code pkt_set_length(pkt_t *pkt, const uint16_t length) {
  pkt->length = length;
  return PKT_OK;
}

pkt_status_code pkt_set_timestamp(pkt_t *pkt, const uint32_t timestamp) {
  pkt->timestamp = timestamp;
  return PKT_OK;
}

pkt_status_code pkt_set_crc1(pkt_t *pkt, const uint32_t crc1) {
  pkt->crc1 = crc1;
  return PKT_OK;
}

pkt_status_code pkt_set_crc2(pkt_t *pkt, const uint32_t crc2) {
  pkt->crc2 = crc2;
  return PKT_OK;
}

pkt_status_code pkt_set_payload(pkt_t *pkt, const char *data,
                                const uint16_t length) {
  // veryfing that payload size is valid
  if (length > MAX_PAYLOAD_SIZE) {
    return E_NOMEM;
  }
  pkt_status_code err;
  err = pkt_set_tr(pkt, 0);
  if (err != PKT_OK) {
    return err;
  }
  err = pkt_set_type(pkt, PTYPE_DATA);
  if (err != PKT_OK) {
    return err;
  }
  memcpy(pkt->payload, data, length);
  pkt_set_length(pkt, length);
  uint32_t crc2 = crc32(0, (Bytef *)data, length);
  pkt_set_crc2(pkt, crc2);
  return PKT_OK;
}

ssize_t varuint_decode(const uint8_t *data, const size_t len,
                       uint16_t *retval) {
  uint8_t header = *data;
  if (header >> 7 == 0) {
    *retval = (uint16_t)*data;
    return 1;
  }
  if (len == 1) {
    return -1;
  }
  uint16_t rval;
  memcpy(&rval, data, 2);
  rval = htons(rval);
  rval &= ~(1 << 15);
  memcpy(retval, &rval, 2);
  return 2;
}

ssize_t varuint_encode(uint16_t val, uint8_t *data, const size_t len) {
  if (val > 32768) {
    return -1;
  }
  if ((size_t)varuint_predict_len(val) > len) {
    return -1;
  }
  if (val < 128) {
    *data = val;
    return 1;
  }
  val |= 1 << 15;
  val = htons(val);
  memcpy(data, &val, 2);
  return 2;
}

size_t varuint_len(const uint8_t *data) {
  uint8_t val = (*data);
  if (val >> 7 == 0) {
    return 1;
  }
  return 2;
}

ssize_t varuint_predict_len(uint16_t val) {
  if (val < 128) {
    return 1;
  } else if (val < 32768) {
    return 2;
  }
  return -1;
}

ssize_t predict_header_length(const pkt_t *pkt) {
  if ((pkt->length) < 128) {
    return 7;
  }
  return 8;
}