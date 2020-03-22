#ifndef __UDP_SENDER_H__
#define __UDP_SENDER_H__

#include <rte_udp.h>
#include <rte_ether.h>
#include <rte_ip.h>

//定义报文信息
struct Message {
	char data[10];
};


#pragma pack(1)
struct Packet
{
    struct rte_ether_hdr ether_hdr;
    struct rte_ipv4_hdr ipv4_hdr;
    struct rte_udp_hdr udp_hdr;
    struct Message msg;
    struct rte_vlan_hdr vlan_hdr;
};

#pragma pop()
#endif // !_UDP_SENDER_H__