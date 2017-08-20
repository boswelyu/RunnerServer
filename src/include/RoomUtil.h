#ifndef _ROOM_UTIL_H_
#define _ROOM_UTIL_H_

class RoomUtil
{
public:
    static bool CompareSockaddr(struct sockaddr * addr1, int len1, struct sockaddr * addr2, int len2)
    {
    	if(len1 != len2) {
    		return false;
    	}

    	if(addr1->sa_family != addr2->sa_family)
    	{
    		return false;
    	}

    	if(memcmp(addr1->sa_data, addr2->sa_data, len1 - 4) == 0)
    	{
        	return true;
        }
        return false;
    }

};
#endif
