#ifndef _THREAD_ADAPT_H_
#define _THREAD_ADAPT_H_
#include <oal.h>
class thread_data
{
public:
	oal_int32 data;
	thread_data()
	{
		data = -1;
	}

};
class thread_adapt //线程函数适配类
{
	public:
		virtual ~thread_adapt() {};
		virtual void process(thread_data* data)
		{
			oal_print("thread processing");
		};
};
class thread_event //线程池队列元素类
{
public:
	thread_adapt* obj;
	thread_data   data;
	bool          need_delete;
	thread_event(thread_adapt* obj1, thread_data  data1, oal_bool  del_flag)
	{
		obj = obj1;
		data = data1;
		need_delete = del_flag;
	}
};
#endif
