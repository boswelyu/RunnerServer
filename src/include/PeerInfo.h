#ifndef _PEER_INFO_H_
#define _PEER_INFO_H_

#include <string.h>

typedef struct peerInfo PeerInfo;

struct peerInfo
{
    struct sockaddr peeraddr;
    int addr_len;

    peerInfo(struct sockaddr * addr, int len)
    {
        memcpy(&peeraddr, addr, sizeof(struct sockaddr));
        addr_len = len;
    }
};

#endif
