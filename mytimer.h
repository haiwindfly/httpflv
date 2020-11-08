#ifndef _MY_TIMER_H_
#define _MY_TIMER_H_
#include <oal.h>
//定时器有不加锁的方案，就是将定时器的添加和删除都放到信号处理函数里做
enum Time_operate
{
    TIMER_ADD = 0,
    TIMER_DEL = 1
};
enum Time_type
{
    ONE_TIME = 0,
    ALL_TIME = 1
};
typedef struct mytimer
{
    oal_bool   is_on;
    Time_type  type;
    oal_uint16 now_time;
    oal_uint16 timeout;
    oal_uint32 sock_fd;
    struct mytimer* next;
    struct mytimer* prev;
}Mytimer;
typedef struct timelist
{
    Mytimer *head;
    Mytimer *tail;
    oal_uint16 num;
    //pthread_spinlock_t lock;//用自旋锁还是互斥锁，在遍历timer链得上锁吧，遍历时间应该很长，所以还是用互斥锁吧
    #ifdef TIMER_LOCK
    pthread_mutex_t g_mutex;
    #endif
}Timerlist;
typedef struct timer_info
{
    Mytimer *time_addr;
    Time_operate OP;
}Timer_info;
extern Timerlist timer_handler;
oal_int16  add_timer(Mytimer *timer1);
oal_int16  set_timeout(Mytimer *timer1,oal_uint16 timeout);
oal_void   reset_time(Mytimer *timer1);
oal_int16  del_timer(Mytimer *timer1);
oal_void   signal_hander();
#endif