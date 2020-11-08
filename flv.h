#ifndef _FLV_H_
#define _FLV_H_
#include <oal.h>
class flv :public thread_adapt
{
public :
	flv() {};
	~flv() {};
	void process(thread_data* data);
};
#endif
void flv::process(thread_data* data)
{
	for (oal_uint8 i = 0;i<data->data; i++)
	{
		oal_print("haige,connect finish!!! \n");
	}
	
}
