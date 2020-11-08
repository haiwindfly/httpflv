#include <threadpool.h>
#include <IO_SERVICE.h>
#include <buffer.h>
#include <mytimer.h>
threadpool::threadpool( my_log* log , oal_uint8 thread_num)
{
	pthread_mutex_init(&queue_mutex, NULL);
	pthread_cond_init(&queue_cond, NULL);
	data_queue = new queue<thread_event>();
	log_t = log;
	thread_head = new pthread_t[thread_num];
	pthread_num = thread_num;
	for (oal_uint8 i = 0;i<thread_num;i++)
	{
		if (pthread_create(thread_head+i,NULL, work_func,this) != 0)//在c++里pthread_create的函数必须是静态函数，此处将对象指针this作为函数参数
		{
			oal_print("create pthread failed\n");
			break;
		}
		if (pthread_detach(thread_head[i])!=0)//pthread_detach(thread_head[i])表示被创建的线程在结束后自动释放所有资源，不需要主线程去调join回收其资源
		{
			oal_print("datach pthread failed\n");
			break;
		}
	}
	buffer_queue = (Buffer *)calloc(thread_num,sizeof(Buffer));
	if(!buffer_queue)
	{
		oal_print("buffer_queue init failed!!\n");
	}
	for(oal_uint8 i = 0;i<thread_num;i++)
	{
		if(buffer_init(&buffer_queue[i],sizeof(Timer_info),TIMER_BUFFER_MAX_NUM))//11.2
		{
			oal_print("Timer_info buff init failed [%d]\n",i);
		}
		else
		{
			oal_print("thread_pool init success [%d]\n",i);
		}
	}
	//创建定时器线程
	if (pthread_create(&timer_pthread,NULL, timer_work_func,this) != 0)//在c++里pthread_create的函数必须是静态函数，此处将对象指针this作为函数参数
	{
		oal_print("create timer_pthread failed\n");
	}
	if (pthread_detach(timer_pthread)!=0)//pthread_detach(thread_head[i])表示被创建的线程在结束后自动释放所有资源，不需要主线程去调join回收其资源
	{
		oal_print("datach timer_pthread failed\n");
	}
}
threadpool::~threadpool()
{
	delete thread_head;
	delete data_queue;
}
void * threadpool::work_func(void *arg)
{
	oal_uint32 tid = syscall(SYS_gettid);
	pthread_tid_map.insert(pair<oal_uint32,oal_uint8>(tid,thread_count++));
	oal_print("tid : [%u] thread_count : [%d]\n",tid,thread_count);
	threadpool* pool = (threadpool*)arg;
	pool->run();
}
void * threadpool::timer_work_func(void* arg)
{
	threadpool* pool = (threadpool*)arg;
	Timerlist*  timer_root = &timer_handler;
	IO_SERVICE* service1 = service;
	while(1)
	{
		sem_wait(&timer_sem);
		//添加或者删除定时器
		Timer_info  time_element;
		for(oal_uint8 i = 0;i < pool->pthread_num ;i++)
		{
			oal_int8  ret = BUFFER_GET_SUCCESS;
			while(ret == BUFFER_GET_SUCCESS)
			{
				ret = buffer_get(&(buffer_queue[i]),(oal_uint8 *)&time_element);
				if(ret == BUFFER_GET_SUCCESS)
				{
					if(time_element.OP == TIMER_ADD && time_element.time_addr != oal_ptr_null)
					{
						add_timer(time_element.time_addr);
						oal_print("thread [%d] ,add timer fd  [%d]\n",i,time_element.time_addr->sock_fd);
					}
					else
					{
						del_timer(time_element.time_addr);
					}
					
				}
			}
		}
		//
		//oal_print("the timer num [%d]\n",timer_root->num);
		Mytimer *temp_timer = timer_root->head;
		Mytimer *next_timer = timer_root->head;
		while(temp_timer != oal_ptr_null)
		{
			next_timer = temp_timer->next;
			oal_print("fd [%d] timer  ,now [%d] ,timeout [%d]\n",temp_timer->sock_fd,temp_timer->now_time , temp_timer->timeout);
			temp_timer->now_time++;
			if(temp_timer->now_time > (temp_timer->timeout + 2) && temp_timer->type == ONE_TIME)
			{
				oal_print("clear fd [%d] timer  ,now [%d] ,timeout [%d]\n",temp_timer->sock_fd,temp_timer->now_time , temp_timer->timeout);
				del_timer(temp_timer);
				service1->clear_fd(temp_timer->sock_fd);
			}
			temp_timer = next_timer;
		}
		//oal_print("do once timer thread\n");
	}
}
void threadpool::run()
{
	while (1)
	{
		oal_print("thread running\n");
		pthread_mutex_lock(&queue_mutex);
		while (data_queue->empty())
		{
			pthread_cond_wait(&queue_cond, &queue_mutex);
		}
		thread_event events = data_queue->front();
		oal_print("thread running 111111,ddta [%d]\n", events.data.data);
		data_queue->pop();
		pthread_mutex_unlock(&queue_mutex);
		events.obj->process(&events.data);
		if (events.need_delete)
		{
			delete events.obj;
		}
	}
}
void threadpool::task_enqueue(thread_event *data)
{
	pthread_mutex_lock(&queue_mutex);
	data_queue->push(*data);
	pthread_cond_signal(&queue_cond);
	pthread_mutex_unlock(&queue_mutex);
}