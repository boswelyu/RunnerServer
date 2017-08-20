#ifndef _GAME_ROOM_INFO_H_
#define _GAME_ROOM_INFO_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <list>
#include <pthread.h>
#include "PeerInfo.h"
#include "RoomMessage.h"

using namespace std;

typedef enum
{
    GRSTATE_FREE = 0,
    GRSTATE_BUSY = 1,
}GameRoomState;

#define MAX_MSG_LIST_LEN 30       //每帧中处理的消息总长度不应该超过一个MTU的大小(1500)

class GameRoom
{
public:
    GameRoom();
    ~GameRoom();

    GameRoomState GetState() { return m_roomState; }
    void SetState(GameRoomState state) { m_roomState = state; }

    int GetRoomId() { return m_roomId; }

    void SetRoomId(int id) { m_roomId = id; }

    bool JoinRoom(struct sockaddr * addr, int addrlen);

    void ProcessMessage(char * msgbuff, int msglen);

    void Recycle();

    void ForwardMessage(int frameNo);

private:
    int PackageMessage(char * msgbuff, int frameNo, RoomMessage * msgList, int msgCount);

private:
    GameRoomState m_roomState;
    int m_roomId;
    typedef list<PeerInfo*> PeerInfoList;
    PeerInfoList peerlist;

    pthread_mutex_t m_mutex;
    int m_recvMsgIndex;
    RoomMessage * m_recvMsgList;
    int m_sendMsgNum;
    RoomMessage * m_sendMsgList;
};
#endif
