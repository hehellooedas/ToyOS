#include <timer.h>
#include <softirq.h>
#include <printk.h>



unsigned long jiffies = 0;


void timer_init(void)
{
    jiffies = 0;
    register_softirq(0,&do_timer ,NULL );
}


void do_timer(void* data)
{
    color_printk(RED,BLACK ,"(HPET:%#lx)\t",jiffies );
}