#include <your_servlet.h>
#include <http_conn.h>
string errorss("error");
oal_uint8* login_html = "login.html";
oal_int32 test_servlet(void* obj)
{
	FILE* fp; 
	
	// char *test_html2222 = "resource/beauty.mp4";
	// //char *test_html2222 = "resource/login.jpg";
	// char *test_html = test_html1111;
	// http_conn* http_obj = (http_conn*)obj; 
	// if(http_obj->getUrl() == "/resource/beauty.mp4")
	// {
	// 	oal_print("get image\n");
	// 	test_html = test_html2222;
	// }
	http_conn* http_obj = (http_conn*)obj; 
	string urls = http_obj->getUrl() ;
	if(urls.size()==0)
	{
		urls.append(login_html);
	}
	oal_print("the url is : [%s]\n",urls.c_str());
	fp = fopen(urls.c_str(),"rb");// localfile文件名  
	if(fp == oal_ptr_null)
	{
		oal_print("the file is not exsit\n");
		http_obj->write(errorss);
		oal_print("sendok\n");
		return 0;
	}	      
	fseek(fp,0L,SEEK_END); /* 定位到文件末尾 */  
	oal_uint32 flen = ftell(fp); /* 得到文件大小 */  
	oal_print("the file len is [%u]\n",flen);
	oal_uint8* p = (oal_uint8 *)malloc(flen + 1); /* 根据文件大小动态分配内存空间 */  
	if(p==NULL)  
	{  
		fclose(fp);  
		oal_print("the file is not exsit\n");
		http_obj->write(errorss);
		oal_print("sendok\n");
		return 0;
	}  
	fseek(fp,0L,SEEK_SET); /* 定位到文件开头 */  
	fread(p,1,flen,fp); /* 一次性读取全部文件内容 */  
	fclose(fp);
/*
函数名: fread
功  能: 从一个流中读数据
用  法: int fread(void *ptr, int size, int nitems, FILE *stream);
参  数：用于接收数据的地址（ptr） 
        单个元素的大小（size） 
        元素个数（nitems）
        提供数据的文件指针（stream）
返回值：成功读取的元素个数
程序例:
*/
	p[flen]=0; /* 字符串结束标志 */   
   // oal_print("the image len [%u]\n",flen);
	// oal_uint8* html_data;
	// oal_uint32 text_len ;
	// oal_uint32  file_fd = open(test_html ,O_RDWR);

	// if(file_fd<0)
	// {
	// 	html_data = "filed";
	// 	oal_print("cannot open the login.html\n");
	// }
	// else
	// {
	// 	text_len = read(file_fd,);
	// }
	//oal_print("the context :\n %s\n", p);
	if(flen > 1500)
	{
		http_obj->write_byte(p,flen,"video/mpeg4");
		free(p);
	}
	else
	{
		string content(p);
		http_obj->write(content);
		oal_print("sendok\n");
	}
	return 0;
	
}