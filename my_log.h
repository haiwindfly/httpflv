#ifndef _MY_LOG_H_
#define _MY_LOG_H_
#include <oal.h>
class my_log
{
	private:
		oal_uint16			 year;
		oal_uint16			 month;
		oal_uint16			 day;
		pthread_mutex_t      write_mutex;
		int					 fd;
	public:
		my_log() {};
		~my_log() {};
		void log(char* format, ...)
		{
		}
};
#endif