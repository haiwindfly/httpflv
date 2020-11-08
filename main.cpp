#include <oal.h>
#include <threadpool.h>
#include <IO_SERVICE.h>
#include <my_log.h>
#include <mytimer.h>
#include <iostream>
#define  thread_num  4
#define  HTTP_PORT   8080
IO_SERVICE* service = oal_ptr_null;
unordered_map<oal_uint32,oal_uint8> pthread_tid_map;
volatile oal_uint8   thread_count = 0;
sem_t  timer_sem;
oal_uint16  test;
//threadpool* pthread_pool = oal_ptr_null;
void prompt_info(int signum)
{
    sem_post(&timer_sem);
    //oal_print("time go!!\n");
}
// struct sigaction {
//          union{
//            __sighandler_t _sa_handler;
//            void (*_sa_sigaction)(int,struct siginfo *, void *)；
//            }_u
//                     sigset_t sa_mask；
//                    unsigned long sa_flags； 
//                  void (*sa_restorer)(void)；
// }
void init_sigaction(void)  
{  
    struct sigaction tact;
    tact.sa_handler = prompt_info;  //sa_handler 不带参数，而(*_sa_sigaction)带参数，但是_sa_sigaction需要sa_flag 置SA_SIGINFO
    tact.sa_flags = SA_RESTART;  //规定信号处理后对打断的系统调用的操作 ,0和SA_INTERRUPT一样
                        //SA_INTERRUPT:由此信号中断的系统调用不会自动重启动(针对sigaction的XSI默认处理方式不会自动重启) 
                        //SA_RESTART:  由此信号中断的系统调用会自动重启。
                        //https://www.ibm.com/developerworks/cn/linux/l-ipc/part2/index1.html
    sigemptyset(&tact.sa_mask);  //设置要阻塞的信号，被阻塞的信号表示不会打断当前信号
    sigaction(SIGALRM, &tact, NULL);  //第三个参数为
}  
void init_time()   
{  
    memset(&timer_handler,0,sizeof(timer_handler));
    struct itimerval value;  
    value.it_value.tv_sec = 1;  
    value.it_value.tv_usec = 0;  
    value.it_interval = value.it_value;  //it_interval非0表示重复装载
    setitimer(ITIMER_REAL, &value, NULL);  //ITIMER_REAL 发 SIGALRM信号
}  
int main()
{
	register_all_servlet();
    sem_init(&timer_sem,0,0);//需要在定时器线程之前初始化信号量
	my_log* sys_log = new my_log();
	service = new IO_SERVICE();
	service->init(HTTP_PORT, sys_log);
	threadpool pthread_pool(sys_log, thread_num);
	service->set_threadpool(&pthread_pool);
	init_sigaction();
	init_time();
	service->listenloop();
	return 0;
}