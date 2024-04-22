#ifndef __LIB_CONTROL_H
#define __LIB_CONTROL_H

#include <printk.h>
#include <memory.h>
#include <log.h>


/*
 * intel处理器共有6个控制寄存器:cr0,cr1,cr2,cr3,cr4,cr8
 * cr0 : 控制处理器的状态和运行模式
 * cr1 : 保留(在访问时会抛出#UD异常)
 * cr2 : 引起#PF异常的线性地址(由分页机制管理,不在这里查看和设置)
 * cr3 : 记录页目录的物理基地址和属性
 * cr4 : 体系结构拓展功能的使能标志
 * cr8 : 通过cr8来操作TPR(读写访问的任务优先寄存器)
 * 在IA-32e中这几个控制寄存器为64位,只是高32位保留使用
 */





/*  cr0控制CPU的状态和运行模式  */
struct cr0_struct{
    unsigned int
        PE:1,       //Protect Enable开启保护模式
        MP:1,       //Monitor Coprocessor开启wait指令监控
        EM:1,       //Emulation检测x87协处理器
        TS:1,       //Task switched延迟保存浮点寄存器的数据
        ET:1,       //Extension Type检测Intel387 DX协处理器
        NE:1,       //Numeric Error选择x87的错误通知机制
        res_1:10,   //保留
        WP:1,       //Write Protect开启只读页的写保护
        res_2:1,    //保留
        AM:1,       //Alignment Mask数据对齐检测
        res_3:10,   //保留
        NW:1,       //Not Write-Through控制系统内存的写穿机制
        CD:1,       //Cache Disable控制系统内存的缓存机制
        PG:1;       //Paging使能分页管理机制
    unsigned int res;
}__attribute__((packed));



static __attribute__((always_inline))
struct cr0_struct cr0_Get(void)
{
    unsigned long cr0;
    asm volatile (
        "movq %%cr0,%0  \n\t"
        :"=r"(cr0)
        :
        :"memory"
    );
    return *(struct cr0_struct*)&cr0;
}



static __attribute__((always_inline))
void print_cr0_info(void)
{
    struct cr0_struct cr0 = cr0_Get();
    if(cr0.PE)     color_printk(RED,BLACK ,"PE " );
    if(cr0.MP)     color_printk(RED,BLACK ,"MP " );
    if(cr0.EM)     color_printk(RED,BLACK ,"EM " );
    if(cr0.TS)     color_printk(RED,BLACK ,"TS " );
    if(cr0.ET)     color_printk(RED,BLACK ,"ET " );
    if(cr0.NE)     color_printk(RED,BLACK ,"NE " );

    if(cr0.WP)     color_printk(RED,BLACK ,"WP " );

    if(cr0.AM)     color_printk(RED,BLACK ,"AM " );

    if(cr0.NW)     color_printk(RED,BLACK ,"NW " );
    if(cr0.CD)     color_printk(RED,BLACK ,"CD " );
    if(cr0.PG)     color_printk(RED,BLACK ,"PG\n" );    //在调用该函数时分页机制必然已经开启

    //color_printk(RED,BLACK,"cr0=%#x\n",*(unsigned int*)&cr0);
}




static __attribute__((always_inline))
void cr0_Set(struct cr0_struct cr0)
{
    if(cr0.res_1 != 0 || cr0.res_2 != 0 || cr0.res_3 != 0 || cr0.res != 0){
        log_to_screen(ERROR,"cr0 Set ERROR!!!");
    }
    asm volatile(
        "movq %0,%%cr0      \n\t"
        :
        :"a"(*(unsigned long*)&cr0)
    );
}




/*  cr3记录页目录的信息  */
#define control_PWT     0x08        //页写穿标志位(为1则可以写穿否则回绕)
#define control_PCD     0x10        //页禁止缓存标志位(为1则禁止缓存否则允许)
/*  一般来说都是回绕+允许缓存(通常不需要更改)  */
static __attribute__((always_inline))
void print_cr3_info(void)
{
    unsigned long cr3 = (unsigned long)Get_gdt();
    color_printk(RED,BLACK,"cr3 = %#x\n",cr3);
    if(cr3 & control_PWT){
        color_printk(RED,BLACK,"PWT = 1\t");
    }
    if(cr3 & control_PCD){
        color_printk(RED,BLACK,"PCD = 1\t");
    }
    color_printk(RED,BLACK,"\n");
}



/*  cr4总是控制一些功能/指令集的开启或关闭  */
struct cr4_struct{
    unsigned int
        VME:1,      //Virtual 8086 mode extensions
        PVI:1,      //Protected-Mode Virtual Interrupt
        TSD:1,      //Time Stamp Disable 限制RDTSC等指令的使用权限
        DE:1,       //Debugging Extensions使能DR4、DR5调试器
        PSE:1,      //Page Size Extensions允许32位分页模式使用4MB物理页
        PAE:1,      //开启页管理机制的物理地址寻址扩展(允许访问超过4GB的物理地址,把原有的二级页表映射为三级)
        MCE:1,      //开启机器检测异常
        PGE:1,      //开启全局页表功能
        PCE:1,      //限制RDPMC指令的执行权限
        OSFXSB:1,   //限制FXAVE,FXSTORE指令的功能
        OSXMMEXCPT:1,   //允许处理器执行SIMD浮点异常
        res_1:2,    //保留
        VMXE:1,     //开启VMX功能
        SMXE:1,     //开启SMX功能
        res_2:1,    //保留
        FSGSBASE:1, //使能RDFSBASE等指令
        PCIDE:1,    //开启PCID功能
        OSXSAVE:1,  //开启XSAVE、XSTORE等指令的增强功能
        res_3:1,    //保留
        SMEP:1,     //限制超级权限对用户进程的执行
        SMAP:1,     //限制超级权限对用户数据的访问

        res_4:10;   //保留
    unsigned int res;
};



static __attribute__((always_inline))
struct cr4_struct cr4_Get(void)
{
    unsigned long cr4;
    asm volatile (
        "movq %%cr4,%0  \n\t"
        :"=r"(cr4)
        :
        :"memory"
    );
    return *(struct cr4_struct*)&cr4;
}



static __attribute__((always_inline))
void cr4_Set(struct cr4_struct cr4)
{
    if(cr4.res !=0 || cr4.res_1 != 0 || cr4.res_2 !=0 || cr4.res_3 != 0 || cr4.res_4 != 0){
        log_to_screen(ERROR,"cr4 Set ERROR!!!");
    }
    asm volatile(
        "movq %0,%%cr0      \n\t"
        :
        :"a"(*(unsigned long*)&cr4)
    );
}



static __attribute__((always_inline))
void print_cr4_info(void)
{
    struct cr4_struct cr4 = cr4_Get();
    if(cr4.VME)     color_printk(RED,BLACK ,"VME " );
    if(cr4.PVI)     color_printk(RED,BLACK ,"PVI " );
    if(cr4.TSD)     color_printk(RED,BLACK ,"TSD " );
    if(cr4.DE)     color_printk(RED,BLACK ,"DE " );
    if(cr4.PSE)     color_printk(RED,BLACK ,"PSE " );
    if(cr4.PAE)     color_printk(RED,BLACK ,"PAE " );
    if(cr4.MCE)     color_printk(RED,BLACK ,"MCE " );
    if(cr4.PGE)     color_printk(RED,BLACK ,"PGE " );
    if(cr4.PCE)     color_printk(RED,BLACK ,"PCE " );
    if(cr4.OSFXSB)     color_printk(RED,BLACK ,"OSFXSB " );
    if(cr4.OSXMMEXCPT)     color_printk(RED,BLACK ,"OSXMMEXCPT " );

    if(cr4.VMXE)    color_printk(RED,BLACK ,"VMXE " );
    if(cr4.SMXE)    color_printk(RED,BLACK ,"SMXE " );

    if(cr4.FSGSBASE)    color_printk(RED,BLACK ,"FSGSBASE " );
    if(cr4.PCIDE)       color_printk(RED,BLACK ,"PCIDE " );
    if(cr4.OSXSAVE)     color_printk(RED,BLACK ,"OSXSAVE " );

    if(cr4.SMEP)    color_printk(RED,BLACK ,"SMEP " );
    if(cr4.SMAP)    color_printk(RED,BLACK ,"SMAP " );

    color_printk(RED,BLACK ,"\n" );
    //color_printk(RED,BLACK,"cr0=%#x\n",*(unsigned int*)&cr0);
}



#endif // !__LIB_CONTROL_H
