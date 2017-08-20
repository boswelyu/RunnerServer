#include <stdio.h>
#include <arpa/inet.h>
#include "GameRoomManager.h"

#define MAX_ROOM_NUM 5000

int GameRoomManager::s_servsock = -1;

GameRoomManager::GameRoomManager()
{
    m_currRoomIndex = 0;
    m_roomBuffCap = MAX_ROOM_NUM;
    m_gameRoomBuffer = new GameRoom[MAX_ROOM_NUM];
    for(int index = 0; index < MAX_ROOM_NUM; index++)
    {
        m_gameRoomBuffer[index].SetRoomId(index + 1);
    }
}

GameRoomManager::~GameRoomManager()
{
    if(m_gameRoomBuffer != NULL)
    {
        delete []m_gameRoomBuffer;
    }
}

// Search out the game room which bind to the peer address
GameRoom * GameRoomManager::GetGameRoom(const char * peeraddr)
{
    GameRoomDict::iterator roomiter = m_gameRoomDict.find(peeraddr);

    if(roomiter != m_gameRoomDict.end())
    {
        // Found one existing game room
        return roomiter->second;
    }

    return NULL;
}

void GameRoomManager::process_push_message(char * msgbuff, int msglen, int recvlen, struct sockaddr * peeraddr, int peeraddr_len)
{
    if(msglen > recvlen) {
        // TODO: message not valid, push to message buffer
        printf("Error: received broken messages!\n");
        return;
    }

    char addrbuff[64];
    if(GenAddrStrKey(peeraddr, addrbuff, sizeof(addrbuff)) == NULL)
    {
        printf("Generate Address String Key for room search failed \n");
	return;
    }

    GameRoom * room = GetGameRoom((const char *)addrbuff);

    if(room != NULL)
    {
        room->ProcessMessage(msgbuff, msglen);
    }
    else
    {
	int roomid = OpenNewRoom(peeraddr, peeraddr_len, addrbuff);
        if(roomid > 0)
        {
            printf("Join room success: roomid: %d, peer addr: %s\n", roomid, addrbuff);
        }
        else
        {
            printf("Join room failed: %s\n", addrbuff);
        }
    }
}

int GameRoomManager::OpenNewRoom(struct sockaddr* addr, int addrlen, char * addrkey)
{
    GameRoom * room = GetFreeGameRoom();
    if(room == NULL)
    {
        printf("No Free Room Available\n");
        return 0;
    }

    m_gameRoomDict[addrkey] = room;

    room->JoinRoom(addr, addrlen);
    
    return room->GetRoomId();
}

char * GameRoomManager::GenAddrStrKey(struct sockaddr * addr, char * resbuff, int bufflen)
{
    // TODO: Validate resbuff
    struct sockaddr_in * addrin = (struct sockaddr_in *)addr;
    const char *nret = inet_ntop(AF_INET, &addrin->sin_addr, resbuff, bufflen);

    if(nret == NULL)
    {
        printf("Convert Address Failed!\n");
        return NULL;
    }

    int iret = snprintf(resbuff, bufflen, "%s:%d", resbuff, addrin->sin_port);
    resbuff[iret] = 0;
    return resbuff;
}

GameRoom * GameRoomManager::GetFreeGameRoom()
{
    int loopedCount = 0, room_index;
    while(loopedCount < m_roomBuffCap)
    {
        GameRoom & room = m_gameRoomBuffer[m_currRoomIndex];
        if(room.GetState() == GRSTATE_FREE)
        {
	    room_index = m_currRoomIndex;
            m_currRoomIndex = (m_currRoomIndex + 1) % m_roomBuffCap;
            return &m_gameRoomBuffer[room_index];
        }

        loopedCount++;
        m_currRoomIndex = (m_currRoomIndex + 1) % m_roomBuffCap;
    }

    printf("Warning: No Available Room Could be found!\n");
    return NULL;
}

