#ifndef __DEVICE_HPET_H
#define __DEVICE_HPET_H

#include <ptrace.h>
#include <printk.h>
#include <APIC.h>
#include <io.h>


/*
 * HPET(High Precision Event CLock)高精度定时设备
 * 包含7组计时器,可以同时独立运行
 * HPET 可以更高效地处理高度时序敏感的应用程序
 * 专门为现代计算机多核架构设备设计,而8253主要为单核设计
*/


void HPET_init(void);
void HPET_handler(unsigned long nr,unsigned long parameter,struct pt_regs* regs);



#endif