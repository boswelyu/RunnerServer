#ifndef _UDP_HEADER_H_
#define _UDP_HEADER_H_

//size of 20 header
typedef struct _udp_msg_header
{
    unsigned int len;
    unsigned short cmd;
    unsigned short version;
    unsigned int seq;
    unsigned int param;
    unsigned int crc;
}UDPMessageHeader;

enum UDPControlCmd
{
	udp_conn_sync = 1,
	udp_conn_ack = 2,
	udp_conn_push = 4,
	udp_conn_fin = 8,
	udp_conn_resend = 0x10,
	udp_conn_ping = 0x20,
	udp_conn_pong = 0x40,
	udp_conn_unctrl_psh = 0x80,
};

#endif