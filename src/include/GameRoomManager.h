#ifndef _GAME_ROOM_MANAGER_H_
#define _GAME_ROOM_MANAGER_H_

#include <hash_map>
#include "GameRoom.h"

using namespace std;
using namespace __gnu_cxx;

class GameRoomManager
{
public:

    static GameRoomManager& Instance()
    {
        static GameRoomManager instance;
        return instance;
    }

    static void SetServerSocketFd(int sock) { s_servsock = sock; }
    static int  GetServerSocketFd() { return s_servsock; }


    ~GameRoomManager();

    void process_push_message(char * msgbuff, int msglen, int recvlen, struct sockaddr * peeraddr, int peeraddr_len);
    GameRoom * GetGameRoom(const char * addr);
    int OpenNewRoom(struct sockaddr* addr, int addrlen, char * addrkey);

private:
    static int s_servsock;
    
    GameRoomManager();

    char * GenAddrStrKey(struct sockaddr * addr, char *buff, int bufflen);
    GameRoom * GetFreeGameRoom();

    typedef hash_map<const char *, GameRoom*> GameRoomDict;
    GameRoomDict m_gameRoomDict;

    GameRoom * m_gameRoomBuffer;   
    int m_roomBuffCap; 		 // The total free room
    int m_currRoomIndex; 	 // 
};

#endif
