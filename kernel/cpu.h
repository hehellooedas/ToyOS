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



extern struct Basic_set_struct Basic_set;
extern struct Extened_set_struct Extened_set;


/*  ECX拓展指令集  */
struct Extened_set_struct{
    unsigned int
        SSE3:1,         //流式SIMD扩展3
        PCLMULQDQ:1,    // 	PCLMULQDQ指令
        DTES64:1,       //64位调试保存
        MINOTOR:1,      //MONITOR和MWAIT指令
        DS_CPL:1,       //
        VMX:1,          //虚拟机扩展
        SMX:1,          //安全模式扩展
        EST:1,          //增强SpeedStep
        TM2:1,          //温度传感器2
        SSSE3:1,        //流式SIMD扩展3补充指令集(Supplemental)
        CNXT_ID:1,      //一级缓存上下文ID
        SDBG:1,         //芯片调试接口
        FMA:1,          //3操作数融合乘加
        CX16:1,         //CMPXCHG16B指令
        XTPR:1,         //可以禁用发送任务优先级消息
        PDCM:1,         //
        res:1,          //保留
        PCID:1,         //进程上下文标识符
        DCA:1,          //DMA写入的直接缓存访问
        SSE4_1:1,       //流式SIMD扩展4.1
        SSE4_2:1,       //流式SIMD扩展4.2
        X2APIC:1,       //x2APIC支持
        MOVBE:1,        //MOVBE指令
        POPCNT:1,       //popcnt指令
        TSC_DEADLINE:1, //APIC 使用 TSC 截止时间值实现一次性操作
        AES:1,          //AES指令集
        XSAVE:1,        // 	XSAVE, XRESTOR, XSETBV, XGETBV指令
        OSXSAVE:1,      //操作系统启用XSAVE
        AVX:1,          //高级矢量扩展
        F16C:1,         //F16C指令集
        RDRND:1,        //集成随机数发生器
        HYPERVISOR:1;   //当前为虚拟机环境
};






/*  EDX基础指令集  */
struct Basic_set_struct{
    unsigned int
        FPU:1,      //集成FPU
        VME:1,      //虚拟8086模式
        DE:1,       //调试模式
        PSE:1,      //页尺寸拓展
        TSC:1,      //时钟周期计数器
        MSR:1,      //特殊模块寄存器
        PAE:1,      //物理地址拓展
        MCE:1,      //机器检查异常
        CX8:1,      //CMPXCHG8指令
        APIC:1,     //集成高级可编程中断控制器
        res_1:1,    //保留
        SEP:1,      //sysenter和sysexit指令支持
        MTRR:1,     //内存类型范围寄存器
        PGE:1,      //全局分页控制位
        MCA:1,      //机器检查架构
        CMOV:1,     //条件传送指令
        PAT:1,      //页属性表
        PSE_36:1,   //36位页尺寸拓展
        PSN:1,      //处理器序列号
        CLFSH:1,    //CLFUSH指令
        res_2:1,    //保留
        DS:1,       //调试保存
        ACPI:1,     //集成ACPI温控寄存器
        MMX:1,      //多媒体拓展
        FXSR:1,     //fxsave,fxstore指令
        SSE:1,      //流式SIMD拓展
        SSE2:1,     //流式SIMD拓展2
        SS:1,       //自窥探缓存
        HTT:1,      //超线程(Hyper-threading)
        TM:1,       //自控温温度传感器
        IA64:1,     //基于IA64架构的处理器
        PBE:1;      //挂起中断
};




#endif // !__KERNEL_CPU_H
