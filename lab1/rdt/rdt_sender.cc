/*
 * FILE: rdt_sender.cc
 * DESCRIPTION: Reliable data transfer sender.
 * NOTE: This implementation assumes there is no packet loss, corruption, or 
 *       reordering.  You will need to enhance it to deal with all these 
 *       situations.  In this implementation, the packet format is laid out as 
 *       the following:
 *       
 *       |<- 7 bits ->|<- 1 bit ->|<-  3 byte  ->|<-             the rest            ->|<-  4 bytes  ->|
 *       |payload size|end of msg | sequence no  |<-             payload             ->|<-  crc-32   ->|
 *
 *       The first byte of each packet indicates the size of the payload
 *       (excluding this single-byte header)
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "rdt_struct.h"
#include "rdt_sender.h"
#include "rdt_protocol.h"

bool sending_started = false;
const double timeout = 0.3;
const double timer_interval = 0.1;
int nothing = 0;
const int max_nothing = 10;
int seq_no = 0;
int last_seq_no = -1;

class PacketInfo {
public:
    packet *pkt;
    double send_time;
    bool acked;
    PacketInfo(): pkt(NULL), send_time(0.0), acked(false) {}
};

PacketInfo sender_packets[RDT_MAX_SEQ_NO];

/* sender initialization, called once at the very beginning */
void Sender_Init()
{
    fprintf(stdout, "At %.2fs: sender initializing ...\n", GetSimulationTime());
}

/* sender finalization, called once at the very end.
   you may find that you don't need it, in which case you can leave it blank.
   in certain cases, you might want to take this opportunity to release some 
   memory you allocated in Sender_init(). */
void Sender_Final()
{
    fprintf(stdout, "At %.2fs: sender finalizing ...\n", GetSimulationTime());
}

/** increase current sequence number */
void Sender_IncrmentSeq()
{
    // std::cout<< "increment seq" << std::endl;
    seq_no = (seq_no + 1)%(RDT_MAX_SEQ_NO+1);
}

/* construct packet using rdt protocol, add header and footer */
void Sender_ConstructPacket(int payload_size, bool end_of_msg, int seq_no, const char *payload, packet *pkt)
{
    ASSERT(payload_size>=0 && payload_size<=(int)RDT_MAX_PAYLOAD_SIZE);
    ASSERT(seq_no>=0&&seq_no<=RDT_MAX_SEQ_NO);
    ASSERT(payload);
    ASSERT(pkt);
    // std::cout<< "construct packet" << std::endl;

    // constrcut header
    pkt->data[0] = (char)(payload_size << RDT_END_OF_MSG_BITS | end_of_msg);
    memcpy(pkt->data+1, (char*)&seq_no, RDT_SEQ_NO_SIZE);

    // copy payload
    memcpy(pkt->data + RDT_HEADER_SIZE, payload, payload_size);
    // if data can not fill the payload, set 0 to pad.
    memset(pkt->data + RDT_HEADER_SIZE + payload_size, 0, RDT_MAX_PAYLOAD_SIZE - payload_size);

    // construct footer
    crc32_padding(pkt);
}

/* check if a packet is ack packet */
bool Sender_CheckACK(packet *pkt, int *seq_no)
{
    ASSERT(pkt);
    // std::cout<< "check ack" << std::endl;
    
    if(!crc32_check(pkt) || pkt->data[0] >> RDT_END_OF_MSG_BITS) {
        return false;
    }

    *seq_no = *(int*)(pkt->data+1) & ((1 << RDT_SEQ_NO_BITS) - 1);
    return true;
}

/* event handler, called when a message is passed from the upper layer at the 
   sender */
void Sender_FromUpperLayer(struct message *msg)
{
    ASSERT(msg);
    ASSERT(msg->size>=0);

    if(!msg->size) return;

    ASSERT(msg->data);
    // std::cout<< "sender from upper layer" << std::endl;

    if(!sending_started) {
        sending_started = true;
        Sender_StartTimer(timer_interval);
    }

    double current_time = GetSimulationTime();

    int last_payload_size = msg->size % RDT_MAX_PAYLOAD_SIZE;
    if(!last_payload_size) last_payload_size = RDT_MAX_PAYLOAD_SIZE;

    int whole_packets_num = (msg->size - last_payload_size)/RDT_MAX_PAYLOAD_SIZE;
    for(int i = 0; i<whole_packets_num; i++) {
        sender_packets[seq_no].pkt = (packet*)malloc(sizeof(packet));
        ASSERT(sender_packets[seq_no].pkt);
        sender_packets[seq_no].send_time = current_time;
        sender_packets[seq_no].acked = false;
        Sender_ConstructPacket(RDT_MAX_PAYLOAD_SIZE, false, seq_no, msg->data + i*RDT_MAX_PAYLOAD_SIZE, sender_packets[seq_no].pkt);
        Sender_ToLowerLayer(sender_packets[seq_no].pkt);
        Sender_IncrmentSeq();
    }

    sender_packets[seq_no].pkt = (packet*)malloc(sizeof(packet));
    ASSERT(sender_packets[seq_no].pkt);
    sender_packets[seq_no].send_time = current_time;
    sender_packets[seq_no].acked = false;
    Sender_ConstructPacket(last_payload_size, true, seq_no, msg->data + whole_packets_num*RDT_MAX_PAYLOAD_SIZE, sender_packets[seq_no].pkt);
    Sender_ToLowerLayer(sender_packets[seq_no].pkt);
    Sender_IncrmentSeq();
}

/* event handler, called when a packet is passed from the lower layer at the 
   sender */
void Sender_FromLowerLayer(struct packet *pkt)
{
    // std::cout<< "sender from lower layer" << std::endl;

    int seq_no;

    // if it is not a valid ack we ignore it
    if(!Sender_CheckACK(pkt, &seq_no)) return;

    // valid ack, free the space and set the pointer to null
    sender_packets[seq_no].acked = true;
    free(sender_packets[seq_no].pkt);
    sender_packets[seq_no].pkt = NULL;
}

/* event handler, called when the timer expires */
void Sender_Timeout()
{
    // std::cout<< "sender timeout" << std::endl;

    double current_time = GetSimulationTime();
    bool remaining = false;

    for(int i = 0; i< seq_no; i++) {
        // std::cout<< "sender timeout: in for-loop" << std::endl;
        if(!sender_packets[i].acked) {
            remaining = true;
            // haven't recieve ack and time expires, resend
            if(current_time - sender_packets[i].send_time >= timeout) {
                Sender_ToLowerLayer(sender_packets[i].pkt);
            }
        }
    }

    nothing = remaining || last_seq_no != seq_no ? 0 : nothing + 1;
    last_seq_no = seq_no;
    if(nothing < max_nothing) {
        Sender_StartTimer(timer_interval);
    }
}