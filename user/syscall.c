#include <printk.h>
#include "../posix/errno.h"



#define __NR_putstring      1
#define __NR_open           2
#define __NR_close          3
#define __NR_read           4
#define __NR_write          5
#define __NR_lseek          6
#define __NR_fork           7
#define __NR_vfork          8


#define SYSCALL_COMMON(nr,sym)  extern unsigned long sym(void);

#include "syscall.h"

#undef SYSCALL_COMMON


#define SYSCALL_COMMON(nr,sym)  [nr] = sym,


#define SYSTEM_CALL_NR      128

typedef unsigned long (*system_call_t)(void) ;






system_call_t system_call_table[SYSTEM_CALL_NR] = {
    [0 ... (SYSTEM_CALL_NR - 1)] = default_system_call,
#include "syscall.h"
};




