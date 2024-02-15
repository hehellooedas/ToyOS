#ifndef __KERNEL_CPU_H
#define __KERNEL_CPU_H

/*
CPUID汇编指令用于鉴别处理器信息以及支持的功能(仅使用32位)
input:
    EAX:主功能号
    ECX:子功能号
output:
    EAX
    EBX
    ECX
    EDX
基础信息:从0开始(当前CPU:0~0x14)EAX=0时可以查询最大基础功能号
拓展信息:从0x80000000开始(当前CPU:0x80000000~0x80000008)EAX=0x80000000时可以查询最大拓展功能号

*/



#define NR_CPUS   8

static __attribute__((always_inline))
void get_cpuid(unsigned int Mop,unsigned int Sop,unsigned int* a,unsigned int* b,unsigned int* c,unsigned int* d)
{
    asm volatile (
        "cpuid      \n\t"
        :"=a"(*a),"=b"(*b),"=c"(*c),"=d"(*d)
        :"0"(Mop),"2"(Sop)
        :"memory"
    );
}


void cpu_init(void);


#endif // !__KERNEL_CPU_H
