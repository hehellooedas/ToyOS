#ifndef __DEVICE_HPET_H
#define __DEVICE_HPET_H

#include <ptrace.h>
#include <printk.h>
#include <APIC.h>
#include <io.h>


/*
 * HPET(High Precision Event CLock)高精度定时设备
 * 包含8组计时器,可以同时独立运行
 * HPET 可以更高效地处理高度时序敏感的应用程序
 * 专门为现代计算机多核架构设备设计,而8253主要为单核设计
*/



#define HPET_GCAP_ID        0x00    //整体机能寄存器
#define HPET_GEN_CONF       0x10    //整体配置寄存器
#define HPET_GINTR_STA      0x20    //整体中断状态寄存器
#define HPET_MAIN_CNT       0xf0    //主计数器

#define HPET_TIM0_CONF      0x100   //定时器0的配置寄存器
#define HPET_TIM0_COMP      0x108   //定时器0的对比寄存器
#define HPET_TIM1_CONF      0x120   //定时器1的配置寄存器
#define HPET_TIM1_COMP      0x128   //定时器1的对比寄存器
#define HPET_TIM2_CONF      0x140   //定时器2的配置寄存器
#define HPET_TIM2_COMP      0x148   //定时器2的对比寄存器
#define HPET_TIM3_CONF      0x160   //定时器3的配置寄存器
#define HPET_TIM3_COMP      0x168   //定时器3的对比寄存器
#define HPET_TIM4_CONF      0x180   //定时器4的配置寄存器
#define HPET_TIM4_COMP      0x188   //定时器4的对比寄存器
#define HPET_TIM5_CONF      0x1a0   //定时器5的配置寄存器
#define HPET_TIM5_COMP      0x1a8   //定时器5的对比寄存器
#define HPET_TIM6_CONF      0x1c0   //定时器6的配置寄存器
#define HPET_TIM6_COMP      0x1c8   //定时器6的对比寄存器
#define HPET_TIM7_CONF      0x1e0   //定时器7的配置寄存器
#define HPET_TIM7_COMP      0x1e8   //定时器7的对比寄存器



void HPET_init(void);
void HPET_handler(unsigned long nr,unsigned long parameter,struct pt_regs* regs);



#endif