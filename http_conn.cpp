#include <http_conn.h>
#include <zlib.h>//用来zip压缩的
#include <buffer.h>
char http_version[] = "HTTP/1.1";
char http_respon_code[][10] =
{
	"200",
	"401",
	"404",
	"503"
};
char http_respon_dscr[][20] =
{
	"OK",
	"FORBIDDEN",
	"NOT FOUND",
	"SERVER ERROR"
};
LINE_STATE http_conn::parse_line()
{
	last_check_index = check_index;
	while (check_index < read_limit_index)
	{
		if (read_buff[check_index] == '\n' )
		{
			if (read_buff[check_index-1] == '\r')//有必要做这个检查吗？
			{
				check_index++;
				return LINE_OK;
			}
			else
			{
				check_index++;
				return LINE_BAD;
			}
		}
		else
		{
			check_index++;
		}
	}
	return LINE_OPEN;
}
HTTP_CODE http_conn::parse_request_line()
{
	oal_uint8* head = read_buff + last_check_index;
	oal_uint8* temp = head;
	oal_uint8* str = strpbrk(temp," \t");//返回空格或者'\t'所在位置
	if (str == oal_ptr_null)
	{
		return REQUEST_ERROR;
	}
	oal_uint8  len = str - temp;
	method.append(temp, len);
	temp = str + 1;
	str  = strpbrk(temp, " \t");
	*str = '\0';
	oal_print("method : [%s]\n", method.c_str());
	oal_uint8* str1 = str;
	if (str == oal_ptr_null)
	{
		return REQUEST_ERROR;
	}
	len = str - temp;
	URL.append(temp + 1, len - 1);
	str = strpbrk(temp, "s");
	if (str == oal_ptr_null)
	{
		servlet_name.append("testservlet");
	}
	else
	{
		if (*(str+1) == 'e' && *(str + 1) == 'r')
		{
			oal_uint8* p = str;
			while (*(--p) != '/'){}
			oal_uint8* start = p + 1;
			p = str;
			while(*(++p) != '/'){}
			p--;
			oal_uint8 len = p - start;
			servlet_name.append(start,len);
		}
		else
		{
			servlet_name.append("testservlet");
		}
	}
	oal_print("url : [%s]\n", URL.c_str());
	temp = str1 + 1;
	str = strpbrk(temp, "\r\n");
	if (str == oal_ptr_null)
	{
		return REQUEST_ERROR;
	}
	len = str - temp;
	version_of_request.append(temp, len);
	oal_print("version : [%s]\n", version_of_request.c_str());
	check_state = CHECK_HEAD_LINE;
	return REQUEST_OK;
}
HTTP_CODE http_conn::parse_request_header()
{
	oal_uint8* temp = read_buff + last_check_index;
	if (*temp == '\r' && *(temp + 1) == '\n' || *temp == '\n')
	{
		if(response_header.find("Connection") != response_header.end() && response_header["Connection"] == "close" )
		{
			short_connect = true;
		}
		check_state = CHECK_OK;
		return REQUEST_OK;
	}
	oal_uint8* str = strpbrk(temp,":");
	if (str == oal_ptr_null)
	{
		return REQUEST_ERROR;
	}
	oal_uint16 len = str - temp;
	string name(temp,len);
	oal_print("str0 [%s]\n",name.c_str());
	temp = str + 1;
	str = strpbrk(temp, "\r\n");
	len = str - temp;
	//oal_print("len:  [%d]  str [%s]\n",len,temp);

	string value(temp,len);
	//oal_print("str1 [%s]\n",value.c_str());
	response_header.insert(pair<string,string>(name, value));
	return REQUEST_OK;
}
HTTP_CODE http_conn:: parse_content()
{
	HTTP_CODE result = REQUEST_MORE;
	//oal_print("parsecontent 1111111\n");
	while(parse_line() == LINE_OK)
	{
		//oal_print("parsecontent 222222\n");
		if (check_state == CHECK_HEAD_LINE)
		{
			//oal_print("parsecontent 333333\n");
			result = parse_request_header();
		}
		else if (check_state == CHECK_REQUEST_LINE)
		{
			//oal_print("parsecontent 4444444\n");
			result = parse_request_line();
		}
		if (check_state == CHECK_OK)
		{
			//oal_print("parsecontent 55555555\n");
			data = read_buff + check_index;
			return REQUEST_OK;
		}
	}
	oal_print("parsecontent end!!!!!!!!\n");
	return result;
}
oal_void http_conn::read(oal_int32 read_index, oal_bool more_data)
{
	read_limit_index = read_index;
	if (more_data)
	{
		if (check_state != CHECK_OK)
		{
			parse_content();
			oal_print("parse_content done 1111111111111\n");
		}
	}
	else
	{
		if (check_state != CHECK_OK)
		{
			if (parse_content() != REQUEST_OK)
			{
				oal_print("request not correct!!!!\n");
				write_head(2,"text/html");
				read_clr();
				return;
			}
		}
		do_servlet();
		read_clr();
		return;
	}
}
oal_uint8  http_conn::set_responseline(oal_uint8 code_index)//在这里会对写信息做初始化
{
	memset(write_buff,0,sizeof(oal_uint8)* BUFFER_SIZE);
	write_index = 0;
	response_header.clear();
	oal_uint16  len = strlen(http_version);
	memcpy(write_buff + write_index, http_version, len);
	write_index += len;

	*(write_buff + write_index) = '\t';
	write_index++;

	len = strlen(http_respon_code[code_index]);
	memcpy(write_buff + write_index, http_respon_code[code_index], len);
	write_index += len;

	*(write_buff + write_index) = '\t';
	write_index++;

	len = strlen(http_respon_dscr[code_index]);
	memcpy(write_buff + write_index, http_respon_dscr[code_index], len);
	write_index += len;

	*(write_buff + write_index) = '\r';
	write_index++;
	*(write_buff + write_index) = '\n';
	write_index++;
	return 0;
}
oal_uint8  http_conn::set_header(string key, string value)
{
	response_header.insert(pair<string,string>(key, value));
	return 0;
}
oal_uint8  http_conn::write_header()
{
	oal_uint8 len;
	unordered_map<string, string>::iterator iter = response_header.begin();
	while (iter != response_header.end())
	{
		const string &first  = iter->first;
		const string &second = iter->second;
		len = first.size();
		memcpy(write_buff+ write_index, first.c_str(), len);
		write_index += len;

		*(write_buff + write_index) = ':';
		write_index++;

		len = second.size();
		memcpy(write_buff + write_index, second.c_str(), len);
		write_index += len;

		*(write_buff + write_index) = '\r';
		write_index++;
		*(write_buff + write_index) = '\n';
		write_index++;
		iter++;
	}
	*(write_buff + write_index) = '\r';
	write_index++;
	*(write_buff + write_index) = '\n';
	write_index++;
	response_header.clear();
	return 0;
}
oal_uint8  http_conn::write_head(oal_uint8 error_code , string context_type)
{
	set_responseline(error_code);
	set_header("Content-Type",context_type);
	set_header("Server", "ngnix");
	set_header("Cache-Control", "no-cache");
	return 0;
}
oal_uint8  http_conn::write(string &content)
{
	//oal_uint8* test_html1 = "<html><head></head><body>haige ok!!go!!</body></html>";
	//oal_uint8* test_html1 = "haige";
	uLong content_len = content.size()+1;
	oal_print("the out string len [%u]\n",content_len);
	write_head(0,"text/html");
	string lengths = to_string(content_len);
	set_header("Content-Length", lengths);
	write_header();
	oal_print("write content\n");
	if (content_len + write_index > BUFFER_SIZE)
	{
		not_write = true;
		oal_print("content length is too long\n");
		return 1;
	}
	/* gzip压缩
	uLong blen = compressBound(content_len);
	if (compress((unsigned char *)(write_buff + write_index), &blen, (unsigned char *)test_html1, content_len) != Z_OK)
	{
		oal_print("compress failed!\n");
		return 1;
	}
	*/
	memcpy(write_buff + write_index, content.c_str(), content_len);
	write_index += content_len;
	
	return 0;
}

oal_uint8  http_conn::write_byte(oal_uint8* head ,oal_uint32 len ,string context_type)
{
	//uLong content_len = content.size()+1;
	//oal_print("the out string len [%u]\n",content_len);
	write_head(0,context_type);
	string lengths = to_string(len);
	set_header("Content-Length", lengths);
	write_header();
	if (len + write_index > BUFFER_SIZE)
	{
		write_need_free = true;
		dyn_write_head = (oal_uint8 *)malloc(len + write_index);
		if(!dyn_write_head)
		{
			write_need_free = false;
			not_write = true;
			return -1;
		}
		memcpy(dyn_write_head,write_buff,write_index);
		memcpy(dyn_write_head + write_index,head,len);
		write_index += len;
		oal_print("the data len is [%u]\n",write_index);
		return 0;
	}
	memcpy(write_buff + write_index, head, len);
	write_index += len;
	return 0;
}