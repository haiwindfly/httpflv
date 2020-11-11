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
	oal_bool    not_write;
	oal_bool    write_need_free;
	oal_uint8  *dyn_write_head;
	oal_uint8   read_buff[BUFFER_SIZE];
	oal_uint8   write_buff[BUFFER_SIZE];
	oal_uint32  epoll_write_index;
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
		not_write = false;
		write_need_free = false;
		epoll_write_index = 0;
		// if(dyn_write_head)
		// {
		// 	free(dyn_write_head);
		// }
	}
};
#endif