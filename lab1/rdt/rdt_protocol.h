#ifndef _RDT_PROTOCOL_H_
#define _RDT_PROTOCOL_H_

#include "rdt_struct.h"

#define RDT_PAYLOAD_SIZE_BITS 7
#define RDT_END_OF_MSG_BITS 1
#define RDT_SEQ_NO_SIZE 3
#define RDT_SEQ_NO_BITS (RDT_SEQ_NO_SIZE * 8)
#define RDT_MAX_SEQ_NO ((1<<(RDT_SEQ_NO_BITS-4)) - 1)
#define RDT_HEADER_SIZE ((RDT_PAYLOAD_SIZE_BITS + RDT_END_OF_MSG_BITS + RDT_SEQ_NO_BITS) / 8)
#define RDT_FOOTER_SIZE sizeof(unsigned int)
#define RDT_MAX_PAYLOAD_SIZE (RDT_PKTSIZE - RDT_HEADER_SIZE - RDT_FOOTER_SIZE)

void crc32_padding(packet *pkt);
bool crc32_check(packet *pkt);

#endif