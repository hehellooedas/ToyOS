#ifndef __KERNEL_LIB_H
#define __KERNEL_LIB_H

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


#endif // !__KERNEL_LIB_H
