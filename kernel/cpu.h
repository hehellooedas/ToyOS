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



/*  ECX拓展指令集  */
#define SSE3        0x1         //流式SIMD拓展3
#define PCLMULQDQ   0x2         //PCLMULQDQ指令
#define DTES64      0x4         //64位Debug store
#define MONITOR     0x8         //MONITOR和MWAIT指令
#define DS_CPL      0x10        //CPL qualified debug store
#define VMX         0x20        //虚拟机拓展,提供虚拟化支持
#define SMX         0x40        //Safer Mode Extensions安全模式拓展
#define EST         0x80        //增强SpeedStep
#define TM2         0x100       //温度传感器2
#define SSSE3       0x200       //流式SIMD扩展3补充指令集
#define CNXT_ID     0x400       //一级缓存上下文ID
#define SDBG        0x800       //Silicon Debug interface
#define FMA         0x1000      //操作数融合乘加
#define CX16        0x2000      //CMPXCHG16B指令
#define XTPR        0x4000      //Can disable sending task priority messages
#define PDCM        0x8000      //Perfmon & debug capability
#define PCID        0x20000     //Process context identifiers
#define DCA         0x40000     //Direct cache access for DMA writes
#define SSE4_1      0x80000     //流式SIMD扩展4.1
#define SSE4_2      0x100000    //流式SIMD扩展4.2
#define X2APIC      0x200000    //x2APIC
#define MOVBE       0x400000    //MOVBE指令
#define POPCNT      0x800000    //POPCNT指令
#define TSC_DEADLINE    0x1000000   //
#define AES         0x2000000   //AES指令集
#define XSAVE       0x4000000   // 	XSAVE, XRESTOR, XSETBV, XGETBV指令
#define OSXSAVE     0x8000000   //操作系统启用XSAVE
#define AVX         0X10000000  //高级矢量拓展
#define F16C        0x20000000  //
#define RDRND       0x40000000  //集成随机数发生器
#define HYPERVISOR  0x80000000  //当前为虚拟机环境

struct extened_set_struct{
    char set_Name[31][12];
    unsigned int set_Value[31];
};


const struct extened_set_struct extened_set = {
    .set_Name = {"SSE3","PCLMULQDQ","DTES64","MONITOR","DS_CPL","VMX","SMX","EST","TM2","SSSE3","CNXT_ID","SDBG","FMA","CX16","XTPR","PDCM","PCID","DCA","SSE4_1","SSE4_2","X2APIC","MOVBE","POPCNT","TSC_DEADLINE","AES","XSAVE","OSXSAVE","AVX","F16C","RDRND","HYPERVISOR"},
    .set_Value = {SSE3,PCLMULQDQ,DTES64,MONITOR,DS_CPL,VMX,SMX,EST,TM2,SSSE3,CNXT_ID,SDBG,FMA,CX16,XTPR,PDCM,PCID,DCA,SSE4_1,SSE4_2,X2APIC,MOVBE,POPCNT,TSC_DEADLINE,AES,XSAVE,OSXSAVE,AVX,F16C,RDRND,HYPERVISOR},
};




/*  EDX基础指令集  */
#define FPU     0x1         //浮点运算单元
#define VME     0x2         //虚拟8086模式拓展
#define DE      0x4         //调试拓展
#define PSE     0x8         //页尺寸拓展(分页机制中可以打开PSE)
#define TSC     0x10        //Time Stamp Counter时钟周期计数器
#define MSR     0x20        //MSR支持(Model-Specific Register，特殊模块寄存器)
#define PAE     0x40        //Physical Address Extension，物理地址扩展
#define MCE     0x80        //机器检查异常
#define CX8     0x100       //CMPXCHG8指令
#define APIC    0x200       //APIC支持
#define SEP     0x800       //sysenter和sysexit支持
#define MTRR    0x1000      //Memory Type Range Registers内存类型范围寄存器
#define PGE     0x2000      //全局分页控制位
#define MCA     0x4000      //机器检查架构
#define CMOV    0x8000      //条件传送指令CMOV
#define PAT     0x10000     //页属性表
#define PSE_36  0x20000     //36位页尺寸扩展
#define PSN     0x40000     //处理器的序列号
#define CLFSH   0x80000     //清除CPU缓存(CLFUSH)
#define DS      0x200000    //调试保存
#define ACPI    0x400000    //集成ACPI温控寄存器
#define MMX     0x800000    //多媒体拓展
#define FXSR    0x1000000   //fxsave,fxrestore指令支持
#define SSE     0x2000000   //流式SIMD拓展
#define SSE2    0x4000000   //流式SIMD拓展2
#define SS      0x8000000   //自窥探缓存
#define HTT     0x10000000  //超线程支持
#define TM      0x20000000  //自控温度传感器
#define IA64    0x40000000  //基于IA64架构的处理器
#define PBE     0x80000000  //挂起中断



struct basic_set_struct{
    char set_Name[30][6];
    unsigned int set_Value[30];
};

const struct basic_set_struct basic_set = {
    .set_Name = {"FPU","VME","DE","PSE","MSR","PAE","MCE","CX8","APIC","SEP","MTRR","PGE","MCA","CMOV","PAT","PSE_36","PSN","CLFSH","DS","ACPI","MMX","FXSR","SSE","SSE2","SS","HTT","TM","IA64","PBE"},
    .set_Value = {FPU,VME,DE,PSE,TSC,MSR,PAE,MCE,CX8,APIC,SEP,MTRR,PGE,MCA,CMOV,PAT,PSE_36,PSN,CLFSH,DS,ACPI,MMX,FXSR,SSE,SSE2,SS,HTT,TM,IA64,PBE},
};




#endif // !__KERNEL_CPU_H
