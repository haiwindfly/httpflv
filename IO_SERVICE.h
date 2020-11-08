#ifndef _IO_SERVICE_H_
#define _IO_SERVICE_H_
#include <oal.h>
#include <http_conn.h>
#include <my_log.h>
#include <thread_adapt.h>
#include <threadpool.h>
#define  MAX_CONN_NUM   4096
#define  MAX_EPOLL_NUM  1000

//IO模式采用的是reactor模式，并发模式为半反应堆（通过队列传递数据，只建立一个epoll的半同步半异步方式）
class IO_SERVICE : public thread_adapt
{
 private :
	oal_uint16  http_port;
	oal_uint16  rtsp_port;
	struct epoll_event events[MAX_EVENT_NUMBER];
	http_conn   http_con[MAX_CONN_NUM];
	oal_uint32  epoll_fd;
	oal_uint32  listen_fd;
	my_log *	log_t;
	threadpool* pthreadpool;
public :
	IO_SERVICE() 
	{
		http_port = 0;
		rtsp_port = 0;
	};
	~IO_SERVICE() {};
	void process(thread_data* data);
	void listenloop();
	void init(oal_uint16 http_port,my_log* log_obj);
	static void addfd(oal_uint32 epollfd,oal_uint32 fd,oal_uint8 oneshot_flag);
	static void modfd(oal_uint32 epollfd, oal_uint32 fd, oal_uint8 oneshot_flag, oal_uint32 ev);
	static void close_fd(oal_uint32 epollfd, oal_uint32 fd);
	static oal_uint32 setnonblocking(oal_uint32 fd);
	threadpool* get_threadpool();
	void set_threadpool(threadpool* pool);
	void clear_fd(oal_uint32 sock_fd)
	{
		http_con[sock_fd].clear();
		close_fd(epoll_fd, sock_fd);
	}
};
extern  IO_SERVICE* service;
#endif
