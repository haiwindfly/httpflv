#ifndef _OAL_H_
#define  _OAL_H_
#define _GNU_SOURCE  1
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>
#include <pthread.h>
#include <queue>
#include <string.h>
#include <unordered_map>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <semaphore.h>
using namespace std;

typedef    unsigned long long		oal_uint64;
typedef    long long				oal_int64;
typedef    int						oal_int32;
typedef    unsigned int				oal_uint32;
typedef    unsigned short			oal_uint16;
typedef    short					oal_int16;
typedef    char						oal_uint8;
typedef    char						oal_int8;
typedef    bool                     oal_bool;
typedef    void                     oal_void;
#define    oal_ptr_null             0x00000000
#define    BUFFER_SIZE              1460
#define    MAX_EVENT_NUMBER         65535
#define    oal_print(format, arg...)				do{printf("%s :  " format, __FUNCTION__,  ##arg);}while(0)
#define    TIMER_BUFFER_MAX_NUM     1024

oal_void register_all_servlet();

enum SERVLET_TYPE
{
	TEST = 0,
	MAX_TYPE = 99
};
extern unordered_map<oal_uint32,oal_uint8> pthread_tid_map;
extern volatile oal_uint8   thread_count;
extern sem_t  timer_sem;
#endif 

