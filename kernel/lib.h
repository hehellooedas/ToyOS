#ifndef __KERNEL_LIB_H
#define __KERNEL_LIB_H


/*
lib.h为通用库文件
内核程序都可以引入lib.h
*/


#include <string.h>
#include <stddef.h>
#include <stdarg.h>


#define nop()   asm volatile("nop   \n\t")
#define stop()  asm volatile("jmp . \n\t")




/*  多核处理器有乱序执行机制(先写入内存的指令不一定真的先执行)  */

/*  最全面、保守的屏障,可以阻止读写指令和写读指令间的重排序  */
#define mfence()    asm volatile("mfence    \n\t":::"memory")
/*  保证加载操作 不会重新排序到lfence之前的存储操作之前(load保护)  */
#define lfence()    asm volatile("lfence    \n\t":::"memory")
/*  保证存储操作 不会重新排序到sfence之后的加载操作之前(store保护)  */
#define sfence()    asm volatile("sfence    \n\t":::"memory")



/*  
根据成员找结构体变量
@ptr:成员变量的地址
@type:成员变量所在结构体类型
@member:成员变量名
*/
#define container_of(ptr,type,member)           \
({                                              \
    typeof(((type*)0)->member)* p = (ptr);        \
    (type*)((unsigned long)p - (unsigned long) & (((type*)0)->member)); \
})



/*
wrmsr指令的封装
    ecx:MSR寄存器的地址
    [edx:eax]:要往里面写入的值
*/
static __attribute__((always_inline))
void wrmsr(unsigned long address,unsigned long value)
{
    asm volatile (
        "wrmsr      \n\t"
        :
        :"c"(address),"d"(value >> 32),"a"(value & 0xffffffff)
        :"memory"
    );
}



/*
rdmsr指令的封装
    ecx:MSR寄存器的地址
    [edx:eax]:获取到的值放入两个寄存器中
*/
static __attribute__((always_inline))
unsigned long rdmsr(unsigned long address)
{
    unsigned int edx=0;
    unsigned int eax=0;
    asm volatile (
        "rdmsr  \n\t"
        :"=d"(edx),"=a"(eax)
        :"c"(address)
        :"memory"
    );
    return (unsigned long)(((unsigned long)edx << 32) | (eax));
}



#endif // !__KERNEL_LIB_H
