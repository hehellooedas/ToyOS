#ifndef __KERNEL_SMP_H
#define __KERNEL_SMP_H

#include <task.h>

extern unsigned char _APU_boot_start[];
extern unsigned char _APU_boot_end[];



#define SMP_cpu_id()    (current->cpu_id)

void SMP_init(void);
void Start_SMP(void);
void IPI_0x200(unsigned long nr,unsigned long parameter,struct pt_regs* regs);


#endif