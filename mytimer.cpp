#include <mytimer.h>
Timerlist timer_handler;
oal_int16  add_timer(Mytimer *timer1)
{
    timer1->is_on = true;
    Timerlist *list = &timer_handler;
    #ifdef TIMER_LOCK
    pthread_mutex_lock(&list->g_mutex); 
    #endif
     if(list->head == oal_ptr_null)
     {
        list->head = list->tail = timer1;
        timer1->next = oal_ptr_null;
        timer1->prev = oal_ptr_null;
     }
     else
     {
        list->tail->next = timer1;
        timer1->prev     = list->tail;
         list->tail      = timer1;
     }
     list->num++ ;
    #ifdef TIMER_LOCK
    pthread_mutex_unlock(&list->g_mutex); 
    #endif
    return 0;
}
oal_void   reset_time(Mytimer *timer1)
{
    timer1->now_time = 0;
}
oal_int16  set_timeout(Mytimer *timer1,oal_uint16 timeout)
{
    timer1->timeout = timeout;
    reset_time(timer1);
    return 0;
}
oal_int16  del_timer(Mytimer *timer1)
{
    Timerlist *list = &timer_handler;
    #ifdef TIMER_LOCK
    pthread_mutex_lock(&list->g_mutex); 
    #endif
    if(timer1->next != oal_ptr_null && timer1->prev != oal_ptr_null)
    {
        timer1->next->prev = timer1->prev;
        timer1->prev->next = timer1->next;
    }
    else if(timer1->next == oal_ptr_null && timer1->prev != oal_ptr_null)
    {
        timer1->prev->next = oal_ptr_null;
        list->tail =  timer1->prev;
    }
    else if(timer1->next != oal_ptr_null && timer1->prev == oal_ptr_null)
    {
        timer1->next->prev = oal_ptr_null;
        list->head =  timer1->next;
    }
    else
    {
        list->head = list->tail = oal_ptr_null;
    }
    list->num --;
    timer1->is_on = false;
    #ifdef TIMER_LOCK
    pthread_mutex_unlock(&list->g_mutex);
    #endif
    return 0;
}
oal_void   signal_hander()
{
    
}