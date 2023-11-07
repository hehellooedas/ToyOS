#ifndef __KERNEL_LIB_H
#define __KERNEL_LIB_H

#include <string.h>
#include <stddef.h>
#include <stdarg.h>


#define nop()   asm volatile("nop   \n\t")
#define stop()  asm volatile("jmp . \n\t")




#endif // !__KERNEL_LIB_H
