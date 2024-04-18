#ifndef __TASK_ATOMIC_H
#define __TASK_ATOMIC_H



typedef struct{
    /* 变量设置为volatile 每次读写都必须到内存操作 */
    __volatile__ long value;
} atomic_T;


/*
 * atomic_T为原子变量
 * lock前缀会锁住硬件平台的前端总线
 * 阻止其他处理器核心访问系统内存
*/


#define atomic_read(atomic) ((atomic)->value)  //获取原子变量的值
#define atomic_set(atomic,value)  ((atomic)->value = (value))  //设置原子变量的值




static __attribute__((always_inline)) 
void atomic_add(atomic_T* atomic,long value)
{
    asm volatile (
        "lock addq %1,%0 \n\t"
        :"=m"(atomic->value)
        :"r"(value)
        :"memory"
    );
}


static __attribute__((always_inline)) 
void atomic_sub(atomic_T* atomic,long value)
{
    asm volatile (
        "lock subq %1,%0 \n\t"
        :"=m"(atomic->value)
        :"m"(value)
        :"memory"
    );
}


static __attribute__((always_inline)) 
void atomic_inc(atomic_T* atomic)
{
    asm volatile (
        "lock incq %0"
        :"=m"(atomic->value)
        :"m"(atomic->value)
        :"memory"
    );
}


static __attribute__((always_inline)) 
void atomic_dec(atomic_T* atomic)
{
    asm volatile (
        "lock decq %0"
        :"=m"(atomic->value)
        :"m"(atomic->value)
        :"memory"
    );
}



static __attribute__((always_inline))
void atomic_set_mask(atomic_T* atomic,long mask)
{
    asm volatile (
        "lock orq %1,%0     \n\t"
        :"=m"(atomic->value)
        :"r"(mask)
        :"memory"
    );
}



static __attribute__((always_inline))
void aotomic_clear_mask(atomic_T* atomic,long mask)
{
    asm volatile (
        "lock andq %1,%0     \n\t"
        :"=m"(atomic->value)
        :"r"(mask)
        :"memory"
    );
}



static __attribute__((always_inline))
int atomic_sub_and_test(atomic_T* atomic,int i)
{
    atomic_sub(atomic,i);
    if(atomic_read(atomic) == 0) return 1;
    return 0;
}






#endif // !__TASK_ATOMIC_H
