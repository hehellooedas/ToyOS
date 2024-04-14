#include <lib.h>
#include <list.h>
#include <task.h>
#include <semaphore.h>



/*  信号量的down操作(阻塞)  */
void __down(semaphore_T *semaphore) 
{
    wait_queue_T wait;
    wait_queue_init(&wait,current);         //初始化当前进程的队列项
    current->state = TASK_UNINTERRUPTIBLE;  //设置为不可中断态
    list_add_to_before(&semaphore->wait.wait_list,&wait.wait_list);
    schedule();     //当前进程转让使用权
}


void semaphore_down(semaphore_T* semaphore)
{
    if(atomic_read(&semaphore->counter) > 0){  //如果资源足够,则分配资源
        atomic_dec(&semaphore->counter);
    }else{      //反之,资源不足够则阻塞当前线程
        __down(semaphore);
    }
}



/*  信号量的up操作(唤醒)  */
void __up(semaphore_T* semaphore)
{
    wait_queue_T* wait = container_of(get_List_next(&semaphore->wait.wait_list),wait_queue_T,wait_list);
    list_del(&wait->wait_list); //把要唤醒的进程从等待队列里删除
    wait->task->state = TASK_RUNNING; //设置为运行态
    insert_task_queue(wait->task);  //把进程插入到就绪队列里以待调度
}



void semaphore_up(semaphore_T* semaphore)
{
    if(list_is_empty(&semaphore->wait.wait_list)){  //如果没有任务需要唤醒
        atomic_inc(&semaphore->counter); //则可使用的资源+1
    }else{
        __up(semaphore);  //若有任务需要唤醒,则唤醒任务
    }
}