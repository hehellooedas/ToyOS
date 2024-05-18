#include <printk.h>
#include "../posix/errno.h"
#include "../fs/VFS.h"
#include <lib.h>
#include <task.h>
#include <log.h>
#include <user.h>



#define __NR_putstring      1
#define __NR_open           2
#define __NR_close          3
#define __NR_read           4
#define __NR_write          5
#define __NR_lseek          6
#define __NR_fork           7
#define __NR_vfork          8



long errno = 0;


unsigned long default_system_call(void)
{
    color_printk(RED,BLACK ,"default system call is calling\n" );
    return -ENOSYS;
}




unsigned long sys_putstring(char* string)
{
    color_printk(ORANGE,BLACK ,string);
    return 0;
}




unsigned long sys_open(char* filename,int flags)
{
    char* path = NULL;
    long pathlen = 0;
    long error = 0;
    struct file* filep = NULL;
    struct file** f = NULL;
    int fd = -1;
    path = (char *)kmalloc(PAGE_4K_SIZE,0);
    if(path == NULL){
        log_to_screen(ERROR,"Failed to alloc memory!");
        return -ENOMEM;
    }
    memset(path,0,PAGE_4K_SIZE);
    pathlen = strnlen_user(filename,PAGE_4K_SIZE );
    if(pathlen < 0){
        kfree(path);
        return -EFAULT;
    }else if(pathlen > PAGE_4K_SIZE){
        kfree(path);
        return -ENAMETOOLONG;
    }
    strncpy_from_user(filename,path ,pathlen );
    return 0;
}




unsigned long sys_close(int fd)
{
    struct file* filep = NULL;
    if(fd < -1 || fd > TASK_FILE_MAX){
        return -EBADF;
    }
    filep = current->file_struct[fd];

    if(filep->f_ops && filep->f_ops->close){
        filep->f_ops->close(filep->dentry->dir_inode,filep);
    }

    kfree(filep);
    current->file_struct[fd] = NULL;
    return 0;
}




unsigned long sys_fork()
{
    struct pt_regs* regs = (struct pt_regs*)current->thread->rsp0 - 1;
    return do_fork(regs,0 ,regs->rsp ,0 );
}



unsigned long sys_vfork()
{
    struct pt_regs* regs = (struct pt_regs*)current->thread->rsp0 - 1;
    return do_fork(regs,CLONE_FS | CLONE_VM | CLONE_SIGNAL ,regs->rsp ,0 );
}



unsigned long sys_exit(int exit_code)
{
    return do_exit(exit_code);
}




#define SYSFUNC_DEF(name)       _SYSFUNC_DEF_(name,__NR_##name)
#define _SYSFUNC_DEF_(name,nr)  __SYSFUNC_DEF__(name,nr)
#define __SYSFUNC_DEF__(name,nr)        \
asm (                                   \
".global "#name"            \n\t"       \
".type "#name",@function    \n\t"       \
#name":                     \n\t"       \
"movq $"#nr",%rax           \n\t"       \
"jmp LABEL_SYSCALL          \n\t"       \
);


SYSFUNC_DEF(putstring)
SYSFUNC_DEF(open)
SYSFUNC_DEF(close)
SYSFUNC_DEF(read)
SYSFUNC_DEF(write)
SYSFUNC_DEF(lseek)
SYSFUNC_DEF(fork)
SYSFUNC_DEF(vfork)


asm (
    "LABEL_SYSCALL:          \n\t"
    "pushq %r10            \n\t"
    "pushq %r11            \n\t"

    "leaq sysexit_return_address(%rip),%r10      \n\t"
    "movq %rsp,%r11       \n\t"
    "sysenter               \n\t"
    "sysexit_return_address: \n\t"

    "xchgq %rdx,%r10      \n\t"
    "xchgq %rcx,%r11      \n\t"

    "popq %r11             \n\t"
    "popq %r10             \n\t"

    "cmpq $-0x1000,%rax     \n\t"
    "jb LABEL_SYSCALL_RET   \n\t"
    "movq %rax,errno(%rip)  \n\t"
    "orq $-1,%rax           \n\t"

    "LABEL_SYSCALL_RET:      \n\t"
    "retq                   \n\t"
);