#ifndef __TASK_SPINLOCK_H
#define __TASK_SPINLOCK_H

#include <preempt.h>
#include <task.h>

/*
 * 自旋锁是一种忙式等待锁
 * 当一个进程试图访问无空闲资源的自旋锁时,会进入低功耗等待(pause)
 * 直到有资源空闲时,继续才会运行
 * 使用场景:短时间内持有锁的场景
 * 自旋锁更加底层,适合给中断处理使用
 */


typedef struct{
    volatile unsigned long lock;
} spinlock_T;




static __attribute__((always_inline))
void spin_init(spinlock_T* lock)
{
    lock->lock = 1;
}



static __attribute__((always_inline))
void spin_lock(spinlock_T* lock)
{
    preempt_disable();
    asm volatile (
        "1:             \n\t"
        "lock decq %0   \n\t"
        "jns 3f         \n\t"   //非负数可以离开
        "2:             \n\t"
        "pause          \n\t"   //低功耗空转(改善忙等待循环)
        "cmpq $0,%0     \n\t"   //与0去比较
        "jle 2b         \n\t"   //小于等于0则继续空转
        "jmp 1b         \n\t"   //大于0则重新进入1
        "3:             \n\t"
        :"=m"(lock->lock)
        :
        :"memory"
    );
}



static __attribute__((always_inline))
void spin_unlock(spinlock_T* lock)
{
    asm volatile (
        "movq $1,%0  \n\t"
        :"=m"(lock->lock)
        :
        :"memory"
    );
    preempt_enable();
}



static __attribute__((always_inline))
long spin_trylock(spinlock_T* lock)
{
    unsigned long tmp_value = 0;
    preempt_disable();
    asm volatile (
        "xchgq %0,%1    \n\t"
        :"=q"(tmp_value),"=m"(lock->lock)
        :"0"(0)
        :"memory"
    );
    if(!tmp_value){
        preempt_enable();
    }
    return tmp_value;
}



#endif