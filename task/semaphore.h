#ifndef __TASK_SEMAPHORE_H

#include <list.h>
#include <task.h>
#include <atomic.h>


typedef struct{
    struct List wait_list;
    struct task_struct* task;
} wait_queue_T;



typedef struct{
    atomic_T counter;
    wait_queue_T wait;
} semaphore_T;


#endif // !__TASK_SEMAPHORE_H