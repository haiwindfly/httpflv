#ifndef _HTTP_CONN_H_
#define _HTTP_CONN_H_
#include <oal.h>
#include <mytimer.h>
#include <connect.h>
#include <buffer.h>
typedef oal_int32(*SERVLET_FUNC)(oal_void*);
enum HTTP_CODE
{
	REQUEST_MORE = 0,
	REQUEST_OK,
	REQUEST_ERROR
};
enum HTTP_CHECK
{
	CHECK_REQUEST_LINE = 0,
	CHECK_HEAD_LINE,
	CHECK_OK
};
enum LINE_STATE
{
	LINE_OK = 0,
	LINE_BAD,
	LINE_OPEN
};
extern  SERVLET_FUNC  servlet[MAX_TYPE];
extern unordered_map<string, SERVLET_TYPE> servlet_index_map;
oal_uint8 servlet_register(SERVLET_TYPE n , SERVLET_FUNC func);
oal_void register_all_servlet();
class http_request
{
protected:
	string version_of_request;
	string method;
	string URL;
	unordered_map<string, string> request_header;
	oal_uint8* data_of_request;
	oal_bool short_connect;
public:
	string getversion()
	{
		return version_of_request;
	}
	string getMethod()
	{
		return method;
	}
	string getUrl()
	{
		return URL;
	}
	oal_bool is_short_connect()
	{
		return short_connect;
	}
	string get_header(string key)
	{
		if (request_header.find(key) != request_header.end())
		{
			return request_header[key];
		}
		else
		{
			return oal_ptr_null;
		}
		
	}
	oal_uint8* getdata()
	{
		return data_of_request;
	}
	void req_clear()
	{
		version_of_request.clear();
		method.clear();
		URL.clear();
		request_header.clear();
		data_of_request = oal_ptr_null;
		short_connect = false;
	}
};

class http_conn : public connect ,public http_request
{
private:
	string servlet_name;//决定调用哪个servlet
	oal_int32  check_index;
	oal_int32  last_check_index;
	oal_int32  read_limit_index;
	oal_uint8* data;
	unordered_map<string, string> response_header;
	oal_uint32 write_index;
	oal_bool   need_write_more;
public:
	HTTP_CHECK check_state;
	Mytimer    alive_timer;
	http_conn()
	{
		check_state = CHECK_REQUEST_LINE;
		check_index = 0;
		need_write_more = 0;
	}
	oal_void read(oal_int32 read_index ,oal_bool more_data);
	LINE_STATE parse_line();
    HTTP_CODE  parse_request_line();
	HTTP_CODE  parse_request_header();
	HTTP_CODE  parse_content();
	oal_uint8  set_responseline(oal_uint8 code_index);
	oal_uint8  set_header(string key,string value);
	oal_uint8  write_header();
	oal_uint8  write(string &content);
	oal_uint8  write_head(oal_uint8 error_code , string context_type);
    oal_uint8  write_byte(oal_uint8* head ,oal_uint32 len ,string context_type);
	oal_void   set_alivetime(oal_uint16 timeout ,Time_type type)
	{
		alive_timer.is_on = true;
		alive_timer.next  = 0;
		alive_timer.next  = oal_ptr_null;
		alive_timer.prev  = oal_ptr_null;
		alive_timer.sock_fd  = fd;
		alive_timer.now_time = 0;
		alive_timer.timeout  = timeout;
		alive_timer.type     = type;
	}
	oal_int8  add_timer2list()
	{
		Timer_info obj;
		obj.OP = TIMER_ADD;
		obj.time_addr = &alive_timer;
		oal_uint8 buffer_id = pthread_tid_map[syscall(SYS_gettid)];
		oal_print("add timre buffer bufferid [%d]\n",buffer_id);
		return buffer_put(&buffer_queue[buffer_id],(oal_uint8 *)&obj,sizeof(Timer_info));
	}
	oal_bool  need_write_again()
	{
		return need_write_more;
	}
	oal_void   read_clr()
	{
		req_clear();
		clearReadbuff();
		servlet_name.clear();
		check_state = CHECK_REQUEST_LINE;
		check_index = 0;
		last_check_index = 0;
		read_limit_index = 0;
		data = read_buff;
	}
	oal_uint32 get_writelen()
	{
		return write_index;
	}
	oal_uint8* get_data()
	{
		return data;
	}
	oal_void clear()
	{
		req_clear();
		connect_clear();
		servlet_name.clear();
		check_state = CHECK_REQUEST_LINE;
		check_index = 0;
		last_check_index = 0;
		read_limit_index = 0;
		data = read_buff;
		response_header.clear();
		write_index = 0;
		need_write_more = false;
		memset(&alive_timer,0,sizeof(alive_timer));
	}
	oal_void do_servlet()
	{
		SERVLET_TYPE n = servlet_index_map[servlet_name];
		servlet[n](this);
	}
	
};

#endif