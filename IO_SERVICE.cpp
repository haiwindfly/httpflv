#include <oal.h>
#include <IO_SERVICE.h>
#include <thread_adapt.h>
#include <zlib.h>//用来zip压缩的
#include <flv.h>
void IO_SERVICE::set_threadpool(threadpool* pool)
{
	pthreadpool = pool;
}
threadpool* IO_SERVICE::get_threadpool()
{
	return pthreadpool;
}
 oal_uint32 IO_SERVICE::setnonblocking(oal_uint32 fd)
{
	oal_uint32 old_opt = fcntl(fd, F_GETFL);
	oal_uint32 new_opt = old_opt | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_opt);
	return old_opt;
}
 void IO_SERVICE::addfd(oal_uint32 epollfd, oal_uint32 fd, oal_uint8 oneshot_flag)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR |EPOLLET;//边沿触发
	if (oneshot_flag)
	{
		event.events = event.events | EPOLLONESHOT;
	}
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	setnonblocking(fd);
}
void IO_SERVICE::modfd(oal_uint32 epollfd, oal_uint32 fd, oal_uint8 oneshot_flag, oal_uint32 ev)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = ev | EPOLLRDHUP | EPOLLHUP | EPOLLERR | EPOLLET;
	if (oneshot_flag)
	{
		event.events = event.events | EPOLLONESHOT;
	}
	epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}
 void  IO_SERVICE::close_fd(oal_uint32 epollfd, oal_uint32 fd)
{
	oal_print("fd close ,[%u]\n",fd);
	 epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
	 close(fd);
}
void IO_SERVICE::init(oal_uint16 http_port1, my_log* log_obj)
{
	http_port = http_port1;
	log_t = log_obj;
	listen_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0)
	{
		oal_print("create listen socket failed\n");
	}
	//优雅关闭连接
	struct linger tmp = { 0, 1 };
	setsockopt(listen_fd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
	struct sockaddr_in address;
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(http_port);
	int flag = 1;
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));//不同ip端口可重复使用
	socklen_t len = sizeof(address);
	int ret = bind(listen_fd,(struct sockaddr *) &address, len);
	if (ret < 0)
	{
		oal_print("bind listen socket failed\n");
		exit(-1);
	}
	ret = listen(listen_fd, 5);
	if (ret < 0)
	{
		oal_print(" listen socket listen failed\n");
		exit(-1);
	}
	epoll_fd = epoll_create(MAX_EPOLL_NUM);
	if (epoll_fd < 0)
	{
		oal_print(" epoll_fd failed\n");
		exit(-1);
	}
	addfd(epoll_fd, listen_fd, 0);

}
void IO_SERVICE::process(thread_data* data)
{
	oal_uint32 index = data->data;
	oal_uint32 temp_fd = http_con[index].fd;
	//oal_print("process 111111\n");
	if (http_con[index].rw_state == O_READ)//边缘触发、非阻塞、reactor
	{
		oal_bool read_over = false;
		memset(http_con[index].read_buff, '\0', BUFFER_SIZE);
		oal_int32 data_read = 0;
		oal_int32 read_index = 0;
		while (1)
		{
			data_read = recv(temp_fd, http_con[index].read_buff + read_index, BUFFER_SIZE - read_index, 0);//为什么要减1
			//data_read = recv(temp_fd, http_con[index].read_buff + read_index, 0, 0);//当第三个数为0时，recv返回0
			//oal_print("process 22222 read len [%d] \n", data_read);
			if (data_read == 0)//返回0表示连接已关闭
			{
				oal_print("connect close!!!\n");
				if(http_con[index].is_short_connect())//短连接直接关闭描述符
				{
					http_con[index].clear();
					close_fd(epoll_fd, temp_fd);
				}
				else //长连接
				{
					//
					if(http_con[index].alive_timer.is_on)//如果定时器开了，那么就让定时器去关闭描述符
					{
						http_con[index].alive_timer.now_time = http_con[index].alive_timer.timeout;//手动设置超时
					}
					else //如果没开直接关闭描述符
					{
						http_con[index].clear();
						close_fd(epoll_fd, temp_fd);
					}
				}
				break;
			}
			else if (data_read < 0)
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK)//表示连接没有数据可读，EAGAIN意思是resource unvaluiable
				{
					//oal_print("process 4444 read len [%d] \n", data_read);
					// //gzip压缩
					// uLong slen = read_index;
					// uLong dlen = compressBound(slen);
					// unsigned char * buff =(unsigned char * ) calloc(1500,1);
					// if(!buff)
					// {
					// 	oal_print("buff failed!\n");
					// }
					// if(uncompress(buff, &dlen,(unsigned char *) http_con[index].read_buff, slen) != Z_OK)
					// {
 					// 	oal_print("uncompress failed! ,[%u]\n",dlen);
					// }
					// else
					// {
					// 	oal_print("the request : [%s]!\n",buff);
					// }
					//memcpy(http_con[index]., http_con[index].read_buff, strlen((oal_int8 *)http_con[index].read_buff));
					http_con[temp_fd].read(read_index,false);
					modfd(epoll_fd, temp_fd, 1, EPOLLOUT);
					oal_print(" start_send\n");
					//reset_oneshot(epoll_fd, temp_fd);
					//do it here
					break;
				}
				else if(EINTR == errno)//表明recv被中断打断，需要重新读
				{
					continue;
				}
				else
				{
					oal_print("something wrong happen!!\n");
					break;
				}
			}
			else
			{
				
				char* path = "http.txt";
				oal_int32 fd = open(path , O_RDWR| O_CREAT| O_APPEND);
				write(fd, http_con[index].read_buff,strlen((oal_int8 *)http_con[index].read_buff));
				close(fd);
				oal_print("process request : %s \n", http_con[index].read_buff);
				http_con[temp_fd].read(read_index,true);
				//oal_print("get_content:%s\n", http_con[index].read_buff);
				//memcpy(http_con[index].write_buff, http_con[index].read_buff, strlen(http_con[index].read_buff));
				//modfd(epoll_fd, temp_fd, 1, EPOLLOUT);
				//sleep(5);
			}
			read_index += data_read;
		}

		
	}
	else
	{
		oal_print("write :\n");
		// char *path = "respon.txt";
		// oal_int32 fd = open(path, O_RDWR | O_CREAT | O_TRUNC);
		// write(fd, http_con[index].write_buff, http_con[index].get_writelen());
		// //oal_print("httprespon : %s\n", http_con[index].write_buff);
		// close(fd);
		if(http_con[index].not_write)
		{
			if(!http_con[index].alive_timer.is_on)//如果定时器没开就关闭连接，如果开了就等待定时器去关闭
			{
				http_con[index].clear();
				close_fd(epoll_fd, temp_fd);//不存在timer
			}
			return;
		}
		
		if(http_con[index].write_need_free)
		{
			oal_int32 ret = 0;
			oal_int32 plen = 0;
			do
			{
				oal_print("the left data len [%d]\n",http_con[index].get_writelen() - plen);
				ret = write(temp_fd, http_con[index].dyn_write_head + plen, http_con[index].get_writelen() - plen);
				if(ret < 0 && errno == EPIPE) //连接被对方关闭
				{
					if(!http_con[index].alive_timer.is_on)
					{
						http_con[index].clear();
						close_fd(epoll_fd, temp_fd);//不存在timer
					}
					else
					{
						http_con[index].alive_timer.now_time = http_con[index].alive_timer.timeout;
					}
					break;
				}
				plen += ret;
		    	oal_print("write data len [%d]\n",ret);
			}while(ret != 0);
			free(http_con[index].dyn_write_head);
			http_con[index].write_need_free = false;
		}
		else
		{
			oal_int32 ret = 0;
			oal_int32 plen = 0;
			do
			{
				ret = write(temp_fd, http_con[index].write_buff + plen, http_con[index].get_writelen() - plen);
				if(ret < 0 && errno == EPIPE)
				{
					if(!http_con[index].alive_timer.is_on)
					{
						http_con[index].clear();
						close_fd(epoll_fd, temp_fd);//不存在timer
					}
					else
					{
						http_con[index].alive_timer.now_time = http_con[index].alive_timer.timeout;
					}
					break;
				}
				plen += ret;
		    	oal_print("write data len [%d]\n",ret);
			}while(ret != 0);
			memset(http_con[index].write_buff, 0, BUFFER_SIZE);
		}
		
		if(http_con[index].is_short_connect())//如果是短连接，发送完数据就关闭连接
		{
			http_con[index].clear();
			close_fd(epoll_fd, temp_fd);//不存在timer
		}
		else
		{
			if(http_con[index].need_write_again())//如果需要再发送数据，则继续注册写就绪事件 ,留给chunk用的，chunk报文除了首个报文有http头，其他报文都没有
			{
				modfd(epoll_fd, temp_fd, 1, EPOLLOUT);
			}
			else
			{
				oal_print("add timer\n");
				modfd(epoll_fd, temp_fd, 1, EPOLLIN);
				//开启定时器，一定时间后还没来数据则关闭连接
				if(!http_con[index].alive_timer.is_on)
				{
					http_con[index].set_alivetime(8,ONE_TIME);
					if(http_con[index].add_timer2list() != BUFFER_PUT_SUCCESS)
					{
						oal_print("time buffer put failed\n");
					}
				}
				else
				{
					http_con[index].alive_timer.now_time = 0;
				}
				
			}
		}
	}

}
void IO_SERVICE::listenloop()
{
	while (1)
	{
		//oal_print("epoll wait!!!\n");
		int num = epoll_wait(epoll_fd, events, MAX_EVENT_NUMBER, -1);
		if(num < 0 && errno == EINTR )
		{
			//oal_print("interrupt!!!\n");
			continue;
		}
		for (int i = 0; i < num; i++)
		{
			if (events[i].data.fd == listen_fd)
			{
				struct sockaddr_in address;
				socklen_t cli_len = sizeof(address);
				int tempfd = accept(listen_fd,(struct sockaddr *) &address,&cli_len);
				if (tempfd < 0)
				{
					oal_print("accept failed\n");
					continue;
				}
				//memset(&http_con[tempfd],0,sizeof(http_con[tempfd]));//涉及到复杂数据类型不能简单的memset
				http_con[tempfd].fd = tempfd;
				http_con[tempfd].m_address = address;
				memset(http_con[tempfd].read_buff,0, BUFFER_SIZE);
				memset(http_con[tempfd].write_buff, 0, BUFFER_SIZE);
				addfd(epoll_fd, tempfd, 1);
			}
			else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))//出现 连接关闭,错误，描述符被挂起
			{
				oal_print("epoll close\n");
				if(http_con[events[i].data.fd].alive_timer.is_on)//如果定时器开了，那么就让定时器去关闭描述符
				{
					http_con[events[i].data.fd].alive_timer.now_time = http_con[events[i].data.fd].alive_timer.timeout;//手动设置超时
				}
				else //如果没开直接关闭描述符
				{
					http_con[events[i].data.fd].clear();
					close_fd(epoll_fd, events[i].data.fd);
				}
				// flv* flv_obj = new flv();
				// thread_data obj;
				// obj.data = 5;
				// thread_event events(flv_obj, obj,true);
				// get_threadpool()->task_enqueue(&events);
			}
			else if (events[i].events & EPOLLIN)
			{
				oal_print("epoll in\n");
				thread_data obj;
				obj.data = events[i].data.fd;
				//快超时了不进读逻辑，真正超时时间到了时由定时器清理连接
				if(!http_con[obj.data].alive_timer.is_on  || http_con[obj.data].alive_timer.now_time < http_con[obj.data].alive_timer.timeout)
				{
					http_con[obj.data].rw_state = O_READ;
					thread_event events(this,obj,false);
					if(http_con[obj.data].alive_timer.is_on)
					{
						http_con[obj.data].alive_timer.now_time = 0;
					}
					get_threadpool()->task_enqueue(&events);
				}
			}
			else if (events[i].events & EPOLLOUT)
			{
				oal_print("epoll out\n");
				thread_data obj;
				obj.data = events[i].data.fd;
				http_con[obj.data].rw_state = O_WRITE;
				thread_event events(this, obj, false);
				get_threadpool()->task_enqueue(&events);
			}
			else
			{
				oal_print("something happened unexpected!!\n");
			}
		}
	}
}
