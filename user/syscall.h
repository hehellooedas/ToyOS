#ifndef __USER_SYSCALL_H
#define __USER_SYSCALL


#define __NR_putstring      1
#define __NR_open           2
#define __NR_close          3
#define __NR_read           4
#define __NR_write          5
#define __NR_lseek          6

#define __NR_fork           7
#define __NR_vfork          8



SYSCALL_COMMON(0,default_system_call)
SYSCALL_COMMON(__NR_putstring,sys_putstring)
SYSCALL_COMMON(__NR_open,sys_open)
SYSCALL_COMMON(__NR_close,sys_close)



#endif