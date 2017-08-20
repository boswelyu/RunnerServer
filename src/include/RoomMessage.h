#ifndef _ROOM_MESSAGE_H_
#define _ROOM_MESSAGE_H_

#define MAX_MESSAGE_LEN 40

class RoomMessage
{
private:
	int m_msgLen;
	int m_msgCmd;
	char m_msgPayload[MAX_MESSAGE_LEN];
public:
	RoomMessage()
	{
		m_msgLen = 0;
		m_msgCmd = 0;
		memset(m_msgPayload, 0, sizeof(m_msgPayload));
	}

	RoomMessage(char * msgbuff, int msglen)
	{
		Parse(msgbuff, msglen);
	}

	bool Parse(char * msgbuff, int msglen)
	{
		memset(m_msgPayload, 0, sizeof(m_msgPayload));
		int * mptr = (int *)msgbuff;
		m_msgLen = *mptr;
		mptr++;
		m_msgCmd = *mptr;

		memcpy(m_msgPayload, msgbuff, msglen);
		return true;
	}

	inline int GetMsgLen() { return m_msgLen; }
	inline char * GetMsgPayload() { return m_msgPayload; }

};
#endif