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



static __attribute__((always_inline))
void spin_lock(spinlock_T* lock)
{
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
}


#endif