#ifndef __KERNEL_SCHEDULE_H
#define __KERNEL_SCHEDULE_H

#include <task.h>
#include <list.h>


struct schedule{
    long running_task_count;        //队列中的管理几个pcb
    long CPU_exec_task_jiffies;     //进程可执行的时间片数量
    struct task_struct task_queue;              //队列头
}task_schedule;     //就绪队列


void schedule_init(void);
void schedule(void);
struct task_struct* get_next_task(void);
void instert_task_queue(struct task_struct* task);


#endif