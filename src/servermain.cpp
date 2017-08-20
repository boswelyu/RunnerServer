#include <stdio.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "utils.h"
#include "UDPHeader.h"
#include "GameRoomManager.h"

#define UDP_PORT 41867

void handle_message(char * recvbuff, int nrecv, int servsock, struct sockaddr * peeraddr_ptr, int peeraddr_len);
void process_udp_sync(UDPMessageHeader *ctrl_ptr, int servsock, struct sockaddr * peeraddr, int peeraddr_len);
void process_udp_ack(UDPMessageHeader * ctrl_ptr, int servsock, struct sockaddr * peeraddr, int peeraddr_len);
void process_udp_fin(UDPMessageHeader * ctrl_ptr, int servsock, struct sockaddr * peeraddr, int peeraddr_len);
void process_udp_push(UDPMessageHeader *ctrl_ptr, int servsock, int nrecv, struct sockaddr * peeraddr, int peeraddr_len);
void process_udp_resend(UDPMessageHeader *ctrl_ptr, int servsock, struct sockaddr * peeraddr, int peeraddr_len);
void process_udp_ping(UDPMessageHeader *ctrl_ptr, int servsock, struct sockaddr * peeraddr, int peeraddr_len);
void process_udp_pong(UDPMessageHeader *ctrl_ptr, int servsock, struct sockaddr * peeraddr, int peeraddr_len);
void process_udp_unctrl_psh(UDPMessageHeader *ctrl_ptr, int servsock, struct sockaddr * peeraddr, int peeraddr_len);
void process_udp_unknown_msg(UDPMessageHeader *ctrl_ptr, int servsock, struct sockaddr * peeraddr, int peeraddr_len);


int main(int argc, char * argv[])
{
	int servsock;

	if((servsock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("Create UDP Socket Failed!");
		return -1;
	}

	const char * serverip = "172.16.0.154";

	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(UDP_PORT);
	servaddr.sin_addr.s_addr = inet_addr(serverip);

	char recvbuff[1500];
	struct sockaddr_in peeraddr;
	socklen_t peerlen;
	int nrecv;

	if(bind(servsock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		printf("Bind UDP Server socket failed!\n");
		return -1;
	}

	printf("UDP Server Started on Port: %d\n", UDP_PORT);
	GameRoomManager::SetServerSocketFd(servsock);

	while(1)
	{
		memset(recvbuff, 0, sizeof(recvbuff));
		nrecv = recvfrom(servsock, recvbuff, sizeof(recvbuff), 0, (struct sockaddr *)&peeraddr, &peerlen);

		printf("Received message bytes len: %d\n", nrecv);

		if(nrecv > 0)
		{
			handle_message(recvbuff, nrecv, servsock, (struct sockaddr *)&peeraddr, peerlen);
		}
	}

	close(servsock);
	return 0;
}

void dump_message(UDPMessageHeader * ctrl_ptr)
{
	printf("=============================\n");
	printf("Message Length: %u\n", ctrl_ptr->len);
	printf("Message command: %u\n", ctrl_ptr->cmd);
	printf("Message version: %u\n", ctrl_ptr->version);
	printf("Message seq: %u\n", ctrl_ptr->seq);
	printf("Message param: %u\n", ctrl_ptr->param);
	printf("Message crc: %u\n", ctrl_ptr->crc);
}

// Handle incoming messages
void handle_message(char * recvbuff, int nrecv, int servsock, struct sockaddr * peeraddr_ptr, int peeraddr_len)
{
	if(nrecv < sizeof(UDPMessageHeader))
	{
		printf("Received package is too small: %d\n", nrecv);
		return;
	}

	UDPMessageHeader * control_ptr = (UDPMessageHeader *)recvbuff;

	switch(control_ptr->cmd)
	{
		case udp_conn_sync:
			process_udp_sync(control_ptr, servsock, peeraddr_ptr, peeraddr_len);
			break;
		case udp_conn_ack:
			process_udp_ack(control_ptr, servsock, peeraddr_ptr, peeraddr_len);
			break;
		case udp_conn_push:
			process_udp_push(control_ptr, servsock, nrecv, peeraddr_ptr, peeraddr_len);
			break;
		case udp_conn_fin:
			process_udp_fin(control_ptr, servsock, peeraddr_ptr, peeraddr_len);
			break;
		case udp_conn_resend:
			process_udp_resend(control_ptr, servsock, peeraddr_ptr, peeraddr_len);
			break;
		case udp_conn_ping:
			process_udp_ping(control_ptr, servsock, peeraddr_ptr, peeraddr_len);
			break;
		case udp_conn_pong:
			process_udp_pong(control_ptr, servsock, peeraddr_ptr, peeraddr_len);
			break;
		case udp_conn_unctrl_psh:
			process_udp_unctrl_psh(control_ptr, servsock, peeraddr_ptr, peeraddr_len);
			break;
		default:
			process_udp_unknown_msg(control_ptr, servsock, peeraddr_ptr, peeraddr_len);
			break;
	}
	return;
}

void process_udp_sync(UDPMessageHeader * ctrl_ptr, int sockfd, struct sockaddr * peeraddr_ptr, int peerlen )
{
	char sendbuff[1500] = {0};

	UDPMessageHeader * sync_ack = (UDPMessageHeader *)sendbuff;
	sync_ack->len = sizeof(UDPMessageHeader);
	sync_ack->cmd = udp_conn_ack;
	sync_ack->version = ctrl_ptr->version;
	sync_ack->seq = 0;
	sync_ack->param = ctrl_ptr->param;
	sync_ack->crc = crc32((unsigned char *)sync_ack, sizeof(UDPMessageHeader) - 4);

	sendto(sockfd, sendbuff, sizeof(UDPMessageHeader), 0, peeraddr_ptr, peerlen);
}

void process_udp_ack(UDPMessageHeader * ctrl_ptr, int servsock, struct sockaddr * peeraddr_ptr, int peeraddr_len)
{
}

void process_udp_push(UDPMessageHeader *ctrl_ptr, int servsock, int nrecv, struct sockaddr * peeraddr, int peeraddr_len)
{
	char msgbuff[1500];
	
	int msglen = ctrl_ptr->len - sizeof(UDPMessageHeader);

	GameRoomManager::Instance().process_push_message((char *)ctrl_ptr + sizeof(UDPMessageHeader), msglen, nrecv - sizeof(UDPMessageHeader), peeraddr, peeraddr_len);
}

void process_udp_fin(UDPMessageHeader * ctrl_ptr, int servsock, struct sockaddr * peeraddr_ptr, int peeraddr_len)
{
}

void process_udp_resend(UDPMessageHeader *ctrl_ptr, int servsock, struct sockaddr * peeraddr, int peeraddr_len)
{
}

void process_udp_ping(UDPMessageHeader *ctrl_ptr, int servsock, struct sockaddr * peeraddr_ptr, int peerlen)
{
    char sendbuff[32] = {0};
    UDPMessageHeader * sync_pong = (UDPMessageHeader *)sendbuff;
    sync_pong->len = sizeof(UDPMessageHeader);
    sync_pong->cmd = udp_conn_pong;
    sync_pong->version = ctrl_ptr->version;
    sync_pong->seq = ctrl_ptr->seq;
    sync_pong->param = ctrl_ptr->param;
    sync_pong->crc = crc32((unsigned char *)sync_pong, sizeof(UDPMessageHeader) - 4);

    sendto(servsock, sendbuff, sizeof(UDPMessageHeader), 0, peeraddr_ptr, peerlen);
}

void process_udp_pong(UDPMessageHeader *ctrl_ptr, int servsock, struct sockaddr * peeraddr, int peeraddr_len)
{
}

void process_udp_unctrl_psh(UDPMessageHeader *ctrl_ptr, int servsock, struct sockaddr * peeraddr, int peeraddr_len)
{
}

void process_udp_unknown_msg(UDPMessageHeader *ctrl_ptr, int servsock, struct sockaddr * peeraddr, int peeraddr_len)
{
}

