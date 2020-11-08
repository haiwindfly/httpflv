#ifndef _CONNECT_H_
#define _CONNECT_H_
#include <oal.h>

enum m_state
{   O_READ = 1,
    O_WRITE
};
class connect
{
public :
	sockaddr_in m_address;
	oal_uint8  *chunk_head;
	oal_uint8   read_buff[BUFFER_SIZE];
	oal_uint8   write_buff[BUFFER_SIZE];
	oal_uint32  fd;
	m_state     rw_state;
	void clearReadbuff()
	{
		memset(read_buff,0, BUFFER_SIZE*sizeof(oal_uint8));
	}
	void clearwritebuff()
	{
		memset(write_buff,0, BUFFER_SIZE*sizeof(oal_uint8));
	}
	void connect_clear()
	{
		clearReadbuff();
		clearwritebuff();
		memset(&m_address,0,sizeof(sockaddr_in));
		fd = 0;
		rw_state = O_READ;
	}
};
#endif