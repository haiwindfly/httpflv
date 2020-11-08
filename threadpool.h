#ifndef _THREAD_POLL_H_
#define _THREAD_POLL_H_

#include <thread_adapt.h>
#include <my_log.h>
#include <buffer.h>
class threadpool
{
	private:
		pthread_t*			 thread_head;
		pthread_t			 timer_pthread;
		my_log*				 log_t;
		queue<thread_event>*  data_queue;
		pthread_mutex_t      queue_mutex;
		pthread_cond_t       queue_cond;    
	public:
	    oal_uint8            pthread_num;
		threadpool(my_log* log_temp, oal_uint8 thread_num);
		~threadpool();
		static void * work_func(void* arg);
		static void * timer_work_func(void* arg);
		void run();
		void task_enqueue(thread_event* data);
};
#endif