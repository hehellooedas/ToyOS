#include <cpu.h>
#include <printk.h>



struct Basic_set_struct Basic_set;
struct Extened_set_struct Extened_set;
struct EAX6_ECX0_EAX_set_struct EAX6_ECX0_EAX_set;
struct EAX7_ECX0_EBX_set_struct EAX7_ECX0_EBX_set;
struct EAX7_ECX0_ECX_set_struct EAX7_ECX0_ECX_set;
struct EAX7_ECX0_EDX_set_struct EAX7_ECX0_EDX_set;


void cpu_init(void)
{
    unsigned int Max_Basic_Number, Max_Entend_Number;
    unsigned int CpuFacName[4] = { 0, 0, 0, 0 };
    char FactoryName[17] = { 0 };


    /*  查看CPU品牌标志(制造商)  */
    get_cpuid(0, 0, &CpuFacName[0], &CpuFacName[1], &CpuFacName[2],
        &CpuFacName[3]);
    Max_Basic_Number = CpuFacName[0]; // 最大基础功能号(重要)
    *(unsigned int*)&FactoryName[0] = CpuFacName[1];
    *(unsigned int*)&FactoryName[4] = CpuFacName[3];
    *(unsigned int*)&FactoryName[8] = CpuFacName[2];
    *(unsigned int*)&FactoryName[12] = '\0';
    color_printk(YELLOW, BLACK, "%s\t", FactoryName);


    /*  查看CPU具体型号信息  */
    for (int i = 0x80000002; i < 0x80000005; i++) {
        get_cpuid(i, 0, &CpuFacName[0], &CpuFacName[1], &CpuFacName[2],
            &CpuFacName[3]);
        *(unsigned int*)&FactoryName[0] = CpuFacName[0];
        *(unsigned int*)&FactoryName[4] = CpuFacName[1];
        *(unsigned int*)&FactoryName[8] = CpuFacName[2];
        *(unsigned int*)&FactoryName[12] = CpuFacName[3];
        *(unsigned int*)&FactoryName[16] = '\0';
        color_printk(YELLOW, BLACK, "%s", FactoryName);
    }
    color_printk(YELLOW, BLACK, "\n");


    /*  查看CPU类型、家族、型号等信息和特性(重要)  */
    get_cpuid(1, 0, &CpuFacName[0], &CpuFacName[1], &CpuFacName[2],
        &CpuFacName[3]);
    color_printk(YELLOW, BLACK, "Family Code:%#lx,Extended Family:%#lx,\
Model Number:%#lx,Entended Model:%#lx,Processer Type:%#lx,Stepping ID:%#lx\n",
        (CpuFacName[0] >> 8 & 0xf), (CpuFacName[0] >> 20 & 0xff),
        (CpuFacName[0] >> 4 & 0xf), (CpuFacName[0] >> 16 & 0xf),
        (CpuFacName[0] >> 12 & 0x3), (CpuFacName[0] & 0xf));

    color_printk(YELLOW,BLACK ,"init APIC ID:%d\n",(CpuFacName[1] >> 24 & 0xff) );

    /*  ECX和EDX存储处理器支持的机能信息  */

    /*  基础指令集信息  */
    color_printk(YELLOW,BLACK ,"basic set support:" );
    Basic_set = *(struct Basic_set_struct*)&CpuFacName[3];
    if(Basic_set.FPU)   color_printk(GREEN,BLACK ,"FPU " );
    if(Basic_set.VME)   color_printk(GREEN,BLACK ,"VME " );
    if(Basic_set.DE)    color_printk(GREEN,BLACK ,"DE " );
    if(Basic_set.PSE)   color_printk(GREEN,BLACK ,"PSE " );
    if(Basic_set.TSC)   color_printk(GREEN,BLACK ,"TSC " );
    if(Basic_set.MSR)   color_printk(GREEN,BLACK ,"MSR " );
    if(Basic_set.PAE)   color_printk(GREEN,BLACK ,"PAE " );
    if(Basic_set.MCE)   color_printk(GREEN,BLACK ,"MCE " );
    if(Basic_set.CX8)   color_printk(GREEN,BLACK ,"CX8 " );
    if(Basic_set.APIC)  color_printk(GREEN,BLACK ,"APIC " );
    if(Basic_set.SEP)   color_printk(GREEN,BLACK ,"SEP " );
    if(Basic_set.MTRR)  color_printk(GREEN,BLACK ,"MTRR " );
    if(Basic_set.PGE)   color_printk(GREEN,BLACK ,"PGE " );
    if(Basic_set.MCA)   color_printk(GREEN,BLACK ,"MCA " );
    if(Basic_set.CMOV)  color_printk(GREEN,BLACK ,"CMOV " );
    if(Basic_set.PAT)   color_printk(GREEN,BLACK ,"PAT " );
    if(Basic_set.PSE_36) color_printk(GREEN,BLACK ,"PSE_36 " );
    if(Basic_set.PSN)   color_printk(GREEN,BLACK ,"PSN " );
    if(Basic_set.CLFSH) color_printk(GREEN,BLACK ,"CLFSH " );
    if(Basic_set.DS)    color_printk(GREEN,BLACK ,"DS " );
    if(Basic_set.ACPI)  color_printk(GREEN,BLACK ,"ACPI " );
    if(Basic_set.MMX)   color_printk(GREEN,BLACK ,"MMX " );
    if(Basic_set.FXSR)  color_printk(GREEN,BLACK ,"FXSR " );
    if(Basic_set.SSE)   color_printk(GREEN,BLACK ,"SSE " );
    if(Basic_set.SSE2)  color_printk(GREEN,BLACK ,"SSE2 " );
    if(Basic_set.SS)    color_printk(GREEN,BLACK ,"SS " );
    if(Basic_set.HTT)   color_printk(GREEN,BLACK ,"HTT " );
    if(Basic_set.TM)    color_printk(GREEN,BLACK ,"TM " );
    if(Basic_set.IA64)  color_printk(GREEN,BLACK ,"IA64 " );
    if(Basic_set.PBE)   color_printk(GREEN,BLACK ,"PBE " );




    /*  扩展指令集信息  */
    color_printk(YELLOW,BLACK ,"\nextended set support:" );
    Extened_set = *(struct Extened_set_struct*)&CpuFacName[2];
    if(Extened_set.SSE3)      color_printk(GREEN,BLACK ,"SSE3 " );
    if(Extened_set.PCLMULQDQ) color_printk(GREEN,BLACK ,"PCLMULQDQ " );
    if(Extened_set.DTES64)    color_printk(GREEN,BLACK ,"DTES64 " );
    if(Extened_set.MINOTOR)   color_printk(GREEN,BLACK ,"MONITOR " );
    if(Extened_set.DS_CPL)    color_printk(GREEN,BLACK ,"DS_CPL " );
    if(Extened_set.VMX)       color_printk(GREEN,BLACK ,"VMX " );
    if(Extened_set.SMX)       color_printk(GREEN,BLACK ,"SMX " );
    if(Extened_set.EST)       color_printk(GREEN,BLACK ,"EST " );
    if(Extened_set.TM2)       color_printk(GREEN,BLACK ,"TM2 " );
    if(Extened_set.SSSE3)     color_printk(GREEN,BLACK ,"SSSE3 " );
    if(Extened_set.CNXT_ID)   color_printk(GREEN,BLACK ,"CNXT_ID " );
    if(Extened_set.SDBG)      color_printk(GREEN,BLACK ,"SDBG " );
    if(Extened_set.FMA)       color_printk(GREEN,BLACK ,"FMA " );
    if(Extened_set.CX16)      color_printk(GREEN,BLACK ,"CX16 " );
    if(Extened_set.XTPR)      color_printk(GREEN,BLACK ,"XTPR " );
    if(Extened_set.PDCM)      color_printk(GREEN,BLACK ,"PDCM " );
    if(Extened_set.PCID)      color_printk(GREEN,BLACK ,"PCID " );
    if(Extened_set.DCA)       color_printk(GREEN,BLACK ,"DCA " );
    if(Extened_set.SSE4_1)    color_printk(GREEN,BLACK ,"SSE4_1 " );
    if(Extened_set.SSE4_2)    color_printk(GREEN,BLACK ,"SSE4_2 " );
    if(Extened_set.X2APIC)    color_printk(GREEN,BLACK ,"X2APIC " );
    if(Extened_set.MOVBE)     color_printk(GREEN,BLACK ,"MOVBE " );
    if(Extened_set.TSC_DEADLINE)  color_printk(GREEN,BLACK ,"TSC_DEADLINE " );
    if(Extened_set.AES)       color_printk(GREEN,BLACK ,"AES " );
    if(Extened_set.XSAVE)     color_printk(GREEN,BLACK ,"XSAVE " );
    if(Extened_set.OSXSAVE)   color_printk(GREEN,BLACK ,"OSXSAVE " );
    if(Extened_set.AVX)       color_printk(GREEN,BLACK ,"AVX " );
    if(Extened_set.F16C)      color_printk(GREEN,BLACK ,"F16C " );
    if(Extened_set.RDRND)     color_printk(GREEN,BLACK ,"RDRND " );
    if(Extened_set.HYPERVISOR) color_printk(GREEN,BLACK ,"HYPERVISOR " );


    //color_printk(YELLOW,BLACK ,"\n" );



    /*  CPU可以支持的物理地址和线性地址的位数  */
    get_cpuid(0x80000008, 0, &CpuFacName[0], &CpuFacName[1], &CpuFacName[2],
        &CpuFacName[3]);
    color_printk(YELLOW, BLACK, "Physical Address size:%08d,\
Linear Address size:%08d\n",
        (CpuFacName[0] & 0xff), (CpuFacName[0] >> 8 & 0xff));


    /*  最大基础\拓展功能号  */
    get_cpuid(0x80000000, 0, &CpuFacName[0], &CpuFacName[1], &CpuFacName[2],
        &CpuFacName[3]);
    Max_Entend_Number = CpuFacName[0]; // 最大拓展功能号(重要)
    color_printk(WHITE, BLACK, "Max_Basic_Number = %#x,Max_Entend_Number = %#x\n",
        Max_Basic_Number, Max_Entend_Number);


    /*  查询电源管理功能  */
    get_cpuid(6, 0, &CpuFacName[0], &CpuFacName[1], &CpuFacName[2],
              &CpuFacName[3]);
    EAX6_ECX0_EAX_set = *(struct EAX6_ECX0_EAX_set_struct*)&CpuFacName[0];




    /*  查询CPU支持的先进指令集信息  */
    get_cpuid(7, 0, &CpuFacName[0], &CpuFacName[1], &CpuFacName[2],
              &CpuFacName[3]);
    EAX7_ECX0_EBX_set = *(struct EAX7_ECX0_EBX_set_struct*)&CpuFacName[1];
    if(EAX7_ECX0_EBX_set.RDSEED) color_printk(GREEN,BLACK ,"RDSEED support!\n" );

    EAX7_ECX0_ECX_set = *(struct EAX7_ECX0_ECX_set_struct*)&CpuFacName[2];
    EAX7_ECX0_EDX_set = *(struct EAX7_ECX0_EDX_set_struct*)&CpuFacName[3];



    /*  查询Cache和TLB信息  */
    get_cpuid(2, 0, &CpuFacName[0], &CpuFacName[1], &CpuFacName[2],
              &CpuFacName[3]);
    color_printk(YELLOW,BLACK ,"%#x,%#x,%#x,%#x\n",CpuFacName[0],CpuFacName[1],CpuFacName[2],CpuFacName[3]);
}
