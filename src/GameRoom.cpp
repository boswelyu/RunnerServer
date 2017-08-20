#include <stdio.h>
#include "GameRoomManager.h"
#include "GameRoom.h"
#include "RoomUtil.h"
#include "PeerInfo.h"
#include "UDPHeader.h"
#include "utils.h"

GameRoom::GameRoom()
{
    m_roomState = GRSTATE_FREE;
    pthread_mutex_init(&m_mutex, NULL);

    m_recvMsgIndex = 0;
    m_sendMsgNum = 0;
    m_recvMsgList = new RoomMessage[MAX_MSG_LIST_LEN];
    m_sendMsgList = new RoomMessage[MAX_MSG_LIST_LEN];
}

GameRoom::~GameRoom()
{
    delete []m_recvMsgList;
    delete []m_sendMsgList;
}

void GameRoom::Recycle()
{
    m_roomState = GRSTATE_FREE;
    peerlist.clear();

    m_recvMsgIndex = 0;
    m_sendMsgNum = 0;
}

bool GameRoom::JoinRoom(struct sockaddr * addr, int addrlen)
{
    // Search if the peer has already in peer list
    PeerInfoList::iterator iter;
    for(iter = peerlist.begin(); iter != peerlist.end(); iter++) 
    {
        if(RoomUtil::CompareSockaddr(&((*iter)->peeraddr), (*iter)->addr_len, addr, addrlen))
        {
            // Oeer already in the peer list
            return true;
        }
    }

    peerlist.push_back(new PeerInfo(addr, addrlen));
    return true;
}

void GameRoom::ProcessMessage(char * msgbuff, int msglen)
{
    pthread_mutex_lock(&m_mutex);
    if(m_recvMsgIndex >= MAX_MSG_LIST_LEN)
    {
        printf("Message Comes too fast\n");
        pthread_mutex_unlock(&m_mutex);
        return;
    }
    // Append the message into message buffer
    RoomMessage * roommsg = &m_recvMsgList[m_recvMsgIndex];
    roommsg->Parse(msgbuff, msglen);
    m_recvMsgIndex++;
    pthread_mutex_unlock(&m_mutex);  
}

void GameRoom::ForwardMessage(int frameNo)
{
    if(m_recvMsgIndex == 0)
    {
        // TODO：没有消息可发送，给每个客户端发送一个空消息
        return;
    }

    // 先加锁，把消息从接收缓冲一次性拷贝到发送缓冲，拷贝完成就解锁，慢慢发送
    pthread_mutex_lock(&m_mutex);
    // Copy message from recv buffer to send buffer
    memcpy(m_sendMsgList, m_recvMsgList, sizeof(RoomMessage) * m_recvMsgIndex);
    m_sendMsgNum = m_recvMsgIndex;
    m_recvMsgIndex = 0;
    pthread_mutex_unlock(&m_mutex); 

    char msgbuff[1500];
    // 增加消息头部
    int msglen = PackageMessage(msgbuff, frameNo, m_sendMsgList, m_sendMsgNum);

    // 把消息分发给每个客户端
    PeerInfoList::iterator iter;
    for(iter = peerlist.begin(); iter != peerlist.end(); iter++) 
    {
        struct sockaddr * peeraddr = &((*iter)->peeraddr);
        int socklen = (*iter)->addr_len;
        sendto(GameRoomManager::GetServerSocketFd(), msgbuff, msglen, 0, peeraddr, socklen);
    }
}

int GameRoom::PackageMessage(char * msgbuff, int frameNo, RoomMessage * msgList, int msgCount)
{
    // Reserve space for UDP Header
    int offset = sizeof(UDPMessageHeader);
    int i = 0;
    RoomMessage * msgptr = NULL;
    for(msgptr = msgList, i = 0; i < msgCount; i++)
    {
        memcpy(msgbuff + offset, msgptr->GetMsgPayload(), msgptr->GetMsgLen());
        offset += msgptr->GetMsgLen();
        msgptr++;
    }

    // 填充UDP头部
    UDPMessageHeader * header = (UDPMessageHeader *)msgbuff;
    header->len = offset;
    header->cmd = udp_conn_push;
    header->version = 7;
    // TODO: seq 数值需要考虑如何填充
    header->seq = 0;
    header->param = frameNo;
    header->crc = crc32((unsigned char *)header, sizeof(UDPMessageHeader) - 4);

    return offset;
}
