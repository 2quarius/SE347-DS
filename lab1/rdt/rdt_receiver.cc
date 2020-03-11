/*
 * FILE: rdt_receiver.cc
 * DESCRIPTION: Reliable data transfer receiver.
 * NOTE: This implementation assumes there is no packet loss, corruption, or 
 *       reordering.  You will need to enhance it to deal with all these 
 *       situations.  In this implementation, the packet format is laid out as 
 *       the following:
 *       
 *       |<-  1 byte  ->|<-             the rest            ->|
 *       | payload size |<-             payload             ->|
 *
 *       The first byte of each packet indicates the size of the payload
 *       (excluding this single-byte header)
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>

#include "rdt_struct.h"
#include "rdt_receiver.h"
#include "rdt_protocol.h"

int last_end_of_msg = -1;

class ReceiveInfo {
public:
    bool received;
    bool is_end;
    std::string data;
    ReceiveInfo(): received(false), is_end(false) {}
};

ReceiveInfo receiver_packets[RDT_MAX_SEQ_NO];

/* receiver initialization, called once at the very beginning */
void Receiver_Init()
{
    fprintf(stdout, "At %.2fs: receiver initializing ...\n", GetSimulationTime());

    for(int i = 0; i<RDT_MAX_SEQ_NO; i++) {
        receiver_packets[i].received = false;
    }
}

/* receiver finalization, called once at the very end.
   you may find that you don't need it, in which case you can leave it blank.
   in certain cases, you might want to use this opportunity to release some 
   memory you allocated in Receiver_init(). */
void Receiver_Final()
{
    fprintf(stdout, "At %.2fs: receiver finalizing ...\n", GetSimulationTime());
}

/* parse packet */
bool Receiver_ParsePacket(packet *pkt, int *payload_size, bool *end_of_msg, int *seq_no, char *payload)
{
    // std::cout<< "receiver parse packet" << std::endl;

    if(!crc32_check(pkt)) return false;

    // get payload size
    *payload_size = pkt->data[0] >> 1 & ((1 << RDT_PAYLOAD_SIZE_BITS) - 1);
    if(*payload_size <= 0 || *payload_size > (int)RDT_MAX_PAYLOAD_SIZE) return false;

    // get end of msg
    *end_of_msg = pkt->data[0] & 1;

    // get seqence number
    *seq_no = *(int*)(pkt->data+1) & ((1 << RDT_SEQ_NO_BITS) - 1);

    // get data
    memcpy(payload, pkt->data + RDT_HEADER_SIZE, *payload_size);

    return true;
}

/* construct ack packet */
void Receiver_ConstructACK(int seq_no, packet *pkt)
{
    // std::cout<< "receiver check ack" << std::endl;

    ASSERT(seq_no >= 0 && seq_no <= RDT_MAX_SEQ_NO);
    ASSERT(pkt);

    // add header
    pkt->data[0] = 0;
    memcpy(pkt->data+1, (char*)&seq_no, RDT_SEQ_NO_SIZE);

    // fill payload with all 0
    memset(pkt->data + RDT_HEADER_SIZE, 0, RDT_MAX_PAYLOAD_SIZE);

    // add footer
    crc32_padding(pkt);
}

/* event handler, called when a packet is passed from the lower layer at the 
   receiver */
void Receiver_FromLowerLayer(struct packet *pkt)
{
    // std::cout<< "receiver from lower layer" << std::endl;

    int payload_size;
    bool end_of_msg;
    int seq_no;
    char payload[RDT_MAX_PAYLOAD_SIZE];
    std::string message;
    struct message msg;

    // invalid packet
    if(!Receiver_ParsePacket(pkt, &payload_size, &end_of_msg, &seq_no, payload)) return;

    // return ack to sender
    packet ack;
    Receiver_ConstructACK(seq_no, &ack);
    Receiver_ToLowerLayer(&ack);

    // receive data
    receiver_packets[seq_no].received = true;
    receiver_packets[seq_no].is_end = end_of_msg;
    receiver_packets[seq_no].data = std::string(payload, payload_size);

    // reassemble
    for(int i = last_end_of_msg + 1; i<RDT_MAX_SEQ_NO; i++) {
        if(!receiver_packets[i].received) break;
        if(receiver_packets[i].is_end) {
            message.clear();
            for(int j = last_end_of_msg + 1; j <= i; j++) {
                message += receiver_packets[j].data;
            }
            msg.size = message.size();
            msg.data = (char*)message.data();
            Receiver_ToUpperLayer(&msg);
            last_end_of_msg = i;
        }
    }
}
