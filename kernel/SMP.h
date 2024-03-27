#ifndef __KERNEL_SMP_H
#define __KERNEL_SMP_H



extern unsigned char _APU_boot_start[];
extern unsigned char _APU_boot_end[];


void SMP_init(void);
void Start_SMP(void);

#endif