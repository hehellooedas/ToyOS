#ifndef __DEVICE_TIMER_H
#define __DEVICE_TIMER_H

#include <list.h>
#include <lib.h>


/*  定时任务队列  */
struct timer_list {
    struct List list;
    unsigned long expire_jiffies;   //当jiffies到达expire_jiffers的时候执行定时任务
    void (*func)(void* data);       //定时任务函数
    void* data;                     //函数参数
}timer_list_head;


extern unsigned long jiffies;

void timer_init(void);
void do_timer(void* data);
void init_timer(struct timer_list* timer,void (*func)(void* data),void* data,unsigned long expire_jiffies);
void add_timer(struct timer_list* timer);
void del_timer(struct timer_list* timer);
void test_timer(void* data);


static __attribute__((always_inline))
void busy_sleep(unsigned long second)
{
    unsigned long start_jiffies = jiffies;
    while(jiffies - start_jiffies < second){
        pause();
    }
}



#endif