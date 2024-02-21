#ifndef __LIB_CONTROL_H
#define __LIB_CONTROL_H

#include <printk.h>


/*
intel处理器共有6个控制寄存器:cr0,cr1,cr2,cr3,cr4,cr8
cr0 : 控制处理器的状态和运行模式
cr1 : 保留
cr2 : 引起#PF异常的线性地址
cr3 : 记录页目录的物理基地址和属性
cr4 : 体系结构拓展功能的使能标志
*/


struct cr0_struct{
    char control_Name[11][3];
    unsigned int control_Values[11]; 
};


struct cr4_struct{
    char coltrol_Name[13][5];
    unsigned int control_Values[13];
};


/*  cr0控制CPU的状态和运行模式  */
#define control_PE  0x01    //Protect Enable开启保护模式
#define control_MP  0x02    //Monitor Coprocessor开启wait指令监控
#define control_EM  0x04    //Emulation检测x87协处理器
#define control_TS  0x08    //延迟保存浮点寄存器的数据
#define control_ET  0x10    //不重要
#define control_NE  0x20    //选择x87的错误通知机制
#define control_WP  0x10000 //Write Protect开启只读页的写保护
#define control_AM  0x40000 //数据对齐检测
#define control_NW  0x20000000  //控制系统内存的写穿机制
#define control_CD  0x40000000  //Cache Disable控制系统内存的缓存机制
#define control_PG  0x80000000  //开启分页机制
const struct cr0_struct cr0_data = {
    .control_Name = {"PE","MP","EM","TS","ET","NE","WP","AM","NW","CD","PG"},
    .control_Values = {control_PE,control_MP,control_EM,control_TS,control_ET,control_NE,control_WP,control_AM,control_NW,control_CD,control_PG},
};
static __attribute__((always_inline))
void print_cr0_info(void)
{
    unsigned long cr0;
    asm volatile (
        "movq %%cr0,%0  \n\t"
        :"=r"(cr0)
        :
        :"memory"
    );
    color_printk(RED,BLACK,"cr0 = %#x\n",cr0);
    for(int i=0;i<11;i++){
        if(cr0 & cr0_data.control_Values[i]){
            color_printk(RED,BLACK,"%s = 1\t",cr0_data.control_Name[i]);
        }
    }
    color_printk(RED,BLACK,"\n");
}




/*  cr3记录页目录的信息  */
#define control_PWT     0x08
#define control_PCD     0x10
static __attribute__((always_inline))
void print_cr3_info(void)
{
    unsigned long cr3;
    asm volatile (
        "movq %%cr3,%0  \n\t"
        :"=r"(cr3)
        :
        :"memory"
    );
    color_printk(RED,BLACK,"cr3 = %#x\n",cr3);
    if(cr3 & control_PWT){
        color_printk(RED,BLACK,"PWT = 1\t");
    }
    if(cr3 & control_PCD){
        color_printk(RED,BLACK,"PCD = 1\t");
    }
    color_printk(RED,BLACK,"\n");
}



/*  cr4控制CPU的体系结构  */
static __attribute__((always_inline))
void print_cr4_info(void)
{
    unsigned long cr4;
    asm volatile (
        "movq %%cr4,%0  \n\t"
        :"=r"(cr4)
        :
        :"memory"
    );
    color_printk(RED,BLACK,"cr4 = %#x\n",cr4);
}




#endif // !__LIB_CONTROL_H
