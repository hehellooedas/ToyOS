#include <printk.h>
#include "../posix/errno.h"
#include "../fs/VFS.h"
#include <lib.h>
#include <task.h>
#include <log.h>
#include <user.h>
#include <io.h>
#include "../posix/stdio.h"
#include "../posix/fcntl.h"



#define __NR_putstring      1
#define __NR_open           2
#define __NR_close          3
#define __NR_read           4
#define __NR_write          5
#define __NR_lseek          6
#define __NR_fork           7
#define __NR_vfork          8



#define SYSTEM_REBOOT       (1 << 0)
#define SYSTEM_POWEROFF     (1 << 1)
#define SYSTEM_HALT         (1 << 2)



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
    struct dir_entry* dentry = NULL;
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
    dentry = path_walk(path, 0);
    kfree(path);
    if(dentry != NULL){
        log_to_screen(INFO,"Find the file!");
    }else{
        log_to_screen(WARNING,"Can't find the file!");
        return -ENOENT;
    }
    if(dentry->dir_inode->attribute == FS_ATTR_DIR){
        return -EISDIR;     //open系统调用应当打开的是文件
    }
    filep = (struct file*)kmalloc(sizeof(struct file),0);
    memset(filep,0,sizeof(struct file));
    filep->dentry = dentry;
    filep->mode = flags;
    filep->f_ops = dentry->dir_inode->f_ops;
    if(filep->f_ops != NULL && filep->f_ops->open != NULL){
        error = filep->f_ops->open(dentry->dir_inode,filep);
    }
    if(error != 1){
        kfree(filep);
        return EFAULT;
    }
    if(filep->mode & O_TRUNC){  //把文件截断掉(覆盖)
        filep->dentry->dir_inode->file_size = 0;
    }
    if(filep->mode & O_APPEND){
        filep->position = filep->dentry->dir_inode->file_size;
    }

    /*  打开文件之后还要把文件和进程进行绑定  */
    f = current->file_struct;
    for(int i=0;i<TASK_FILE_MAX;i++){
        if(f[i] == NULL){
            fd = i;
            break;
        }
    }
    if(fd == TASK_FILE_MAX){
        kfree(filep);
        return -ENFILE;
    }

    f[fd] = filep;
    return fd;
}




unsigned long sys_close(int fd)
{
    struct file* filep = NULL;
    if(fd < -1 || fd >= TASK_FILE_MAX){
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




unsigned long sys_read(int fd,void* buf,long count)
{
    struct file* filep = NULL;
    if(fd < 0 || fd >= TASK_FILE_MAX){
        return -EBADF;
    }
    if(count < 0) return -EINVAL;
    filep = current->file_struct[fd];
    if(filep->f_ops != NULL && filep->f_ops->read != NULL){
        filep->f_ops->read(filep,buf,count,&filep->position);
    }
    return 0;
}




unsigned long sys_write(int fd,void * buf,long count)
{
    struct file* filep = NULL;
    if(fd < 0 || fd >= TASK_FILE_MAX){
        return -EBADF;
    }
    if(count < 0) return -EINVAL;
    filep = current->file_struct[fd];
    if(filep->f_ops != NULL && filep->f_ops->write != NULL){
        filep->f_ops->write(filep,buf,count,&filep->position);
    }
    return 0;
}




unsigned long sys_lseek(int fd,long offset,int whence)
{
    struct file* filep = NULL;
    if(fd < 0 || fd >= TASK_FILE_MAX){
        return -EBADF;
    }
    if(whence < 0 || whence > SEEK_MAX){
        return -EINVAL;
    }
    filep = current->file_struct[fd];
    if(filep->f_ops != NULL && filep->f_ops->lseek != NULL){
        filep->f_ops->lseek(filep,offset,whence);
    }
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



unsigned long sys_reboot(unsigned long cmd,void* arg)
{
    switch(cmd){
        case SYSTEM_REBOOT:
            out8(0x64,0xfe);
            break;
        case SYSTEM_POWEROFF:
            log_to_screen(INFO,"System will poweroff.");
            break;
        case SYSTEM_HALT:
            log_to_screen(WARNING,"Syetem will go to halt.");
            while(1)
                hlt();
            break;
        default:
            log_to_screen(ERROR,"ERROR CMD!");
            break;
    }
    return 0;
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