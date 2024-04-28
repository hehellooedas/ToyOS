#include <VFS.h>
#include <string.h>



struct file_system_type filesystem = {"filesystem",0}; //所有注册到VFS中的文件系统都在链上




struct super_block* mount_fs(char* name,struct Disk_Patition_Table_Entry* DPTE,void* buf)
{
    struct file_system_type* p = NULL;
    for(p=&filesystem;p;p=p->next){
        if(!(strcmp(p->name,name ))){   //输出0代表名称相同,名称不同则继续往后找
            return p->read_superblock(DPTE,buf);
        }
    }
    return 0;   //遍历结束还没找到,说明没有该名称的文件系统
}



unsigned long register_filesystem(struct file_system_type* fs)
{
    struct file_system_type* p = NULL;
    for(p=&filesystem;p;p=p->next){
        if(!(strcmp(fs->name,p->name))){
            return 0;
        }
    }
    fs->next = filesystem.next;
    return 1;
}