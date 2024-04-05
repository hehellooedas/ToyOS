#ifndef __TASK_PREEMPT_H
#define __TASK_PREEMPT_H


#define preempt_enable()        \
do{                             \
    current->preempt_count--;   \
}while(0)



#define preempt_disable()        \
do{                             \
    current->preempt_count++;   \
}while(0)



#endif