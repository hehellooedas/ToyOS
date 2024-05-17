

#define __NR_putstring      1
#define __NR_open           2
#define __NR_close          3
#define __NR_read           4
#define __NR_write          5
#define __NR_lseek          6


SYSCALL_COMMON(0,default_system_call)
SYSCALL_COMMON(1,sys_putstring)
SYSCALL_COMMON(2,sys_open)