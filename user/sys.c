#include <printk.h>
#include <errno.h>
#include <VFS.h>
#include <lib.h>


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



unsigned long sys_open()
{
    return 0;
}



unsigned long sys_close(int fd)
{
    struct file* filep = NULL;

    return 0;
}