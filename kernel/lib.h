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
