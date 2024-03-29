#ifndef __TASK_SEMAPHORE_H

#include <list.h>
#include <task.h>
#include <atomic.h>



/*
 * 信号量是一种休眠锁
 * 当一个进程试图访问无空闲资源的信号量时,会进入休眠状态
 * 直到有空闲资源时,会把该进程从等待队列中唤醒
 * 使用场景:控制资源的访问数量
 */


/*  等待队列中的每一个项目  */
typedef struct{
    struct List wait_list;    //用来标记在队列中的位置
    struct task_struct* task; //该进程
} wait_queue_T;



/*  信号量  */
typedef struct{
    atomic_T counter;  //该信号量拥有资源的数量(原子变量)
    wait_queue_T wait; //等待当前资源的任务队列
} semaphore_T;



/*  初始化等待队列  */
static __attribute__((always_inline))
void wait_queue_init(wait_queue_T* wait_queue,struct task_struct* task)
{
    list_init(&wait_queue->wait_list);
    wait_queue->task = task;
}                           



/*  以下是函数声明  */
void semaphore_down(semaphore_T *semaphore);
void semaphore_up(semaphore_T *semaphore);


#endif // !__TASK_SEMAPHORE_H