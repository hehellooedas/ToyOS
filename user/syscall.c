#include <printk.h>
#include "../posix/errno.h"
#include <sys.h>




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




