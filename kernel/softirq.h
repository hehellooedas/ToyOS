#ifndef KERNEL_SOFTIRQ_H
#define KERNEL_SOFTIRQ_H

#include <HPET.h>
#include <interrupt.h>




/*
 * 硬件中断主要是为了处理硬件发过来的中断
 * 但很多时候软件也需要类似“中断”这样的处理方式
 * 硬件中断一般比较重要,越早处理越好;而软件中断是为了实现某些机制,稍微延迟一点去处理也没什么事
 * 每个软中断对应一个位,置位意味着需要处理该软中断
 */


#define TIMER_STRQ  (1 << 0)    //时钟中断是第一个软中断





/*  用于描述软中断的处理方法和参数  */
struct softirq{
    void (*action)(void* data);
    void* data;
};



struct softirq softirq_vector[64] = {0};


void softirq_init(void);
void set_softirq_status(unsigned long status);
unsigned long get_softirq_status(void);
void register_softirq(int nr,void (*action)(void* data),void* data);
void unregister_softirq(int nr);
void do_softirq(void);



#endif