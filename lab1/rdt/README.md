# Lab1 Reliable Data Transport Protocol
- Name: `sixplus`
- Student ID: `517021911048`
- Email: `sixplus@sjtu.edu.cn`
## Corruption
There are two methods error detection and error correction to solve data corruption and I chose `error detection` and `retransmit`. For error detection, there are many ways to achieve the goal, so I just pick up the CRC-32 whose polynomial representation is $x^{32}+x^{26}+x^{23}+x^{22}+x^{16}+x^{12}+x^{11}+x^{10}+x^{8}+x^{7}+x^{5}+x^{4}+x^{2}+x+1$.

## Loss
The sender will start a timer when sending a packet and if reciever recieves this packet he will acknowledge the sender otherwise when time is out, sender resend.

The sender should maintain a buffer to record the packets sent within a message. So I defined a class `PacketInfo` to record these information including sending timestamp, packet data and an ack. And I have a global variable `PacketInfo sender_packets[RDT_MAX_SEQ_NO]` to indicate the packets info I have sent.

When the first packet is to be sent, I start the timer. And if the reciever sends ACK n to sender during the timeout, I will set the ack of corresponding packet's info to true and free the space of that packet; if times out, I will resend the packet which locates at the head and restart the timer accordingly.

## Out-of-order
To solve out-of-order situation, I make the header size larger than original to store the sequence number of each splitted packet and let receiver to reassemble these packets into an ordered message.

## Details
- Because the maximum packet size is 128 bytes, so the payload size will not exceed 128 which is $2^{7}$. Therefore, I use 7 bits to store payload size and one bit remaining to indicate that whether this packet is the end of the whole message.
- Use 3 bytes to indicate the squence number of that packet that is used in receiver side to reassemble mixed packets.
- To indicate one packet is an ack packet not data packet, I let the first one byte of the packet be 0.

## Summary
My implementation is base on selective repeat protocol.