#ifndef __TASK_SPINLOCK_H
#define __TASK_SPINLOCK_H



typedef struct{
    volatile unsigned long lock;
} spinlock_T;



static __attribute__((always_inline))
void spin_init(spinlock_T* lock)
{
    lock->lock = 1;
}

#endif