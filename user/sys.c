#include <printk.h>
#include "../posix/errno.h"
#include "../fs/VFS.h"
#include <lib.h>
#include <task.h>
#include <log.h>
#include <user.h>




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