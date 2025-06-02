#include <VFS.h>
#include <string.h>
#include <log.h>
#include <lib.h>
#include <memory.h>



struct file_system_type filesystem = {"filesystem",0}; //所有注册到VFS中的文件系统都在链上

struct super_block* root_sb;    //保存操作系统的根文件系统信息


struct super_block* mount_fs(char* name,struct Disk_Partition_Table_Entry* DPTE,void* buf)
{
    struct file_system_type* p = NULL;
    for(p=&filesystem;p!=NULL;p=p->next){
        if(!strcmp(p->name,name)){
            return p->read_superblock(DPTE,buf);
        }
    }
    log_to_screen(ERROR,"No filesystem found!");
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
    filesystem.next = fs;
    return 1;
}



unsigned long unregister_filesystem(struct file_system_type* fs){
    struct file_system_type* p = &filesystem;
    while(p->next){
        if(p->next == fs){
            p->next = p->next->next;
            fs->next = NULL;
            return 1;
        }else{
            p = p->next;
        }
    }
    return 0;
}



unsigned check_filesystem(){
    struct file_system_type* p = &filesystem;
    while(p){
        color_printk(BLUE,BLACK,"filesystem's name is %s\n",p->name);
        p = p->next;
    }
    return 1;
}



/*  逐层搜索路径名  */
struct dir_entry* path_walk(char* name,unsigned long flags)
{
    char* tmpname = NULL;
    int tmpnamelen = 0;
    struct dir_entry* parent = root_sb->root;   //不再特指FAT32
    struct dir_entry* path = NULL;

    while(*name == '/'){
        name++;
    }
    if(!*name) return parent;

    for(;;){
        tmpname = name;
        while(*name && (*name != '/')) name++;
        tmpnamelen = name - tmpname;        //确定目录名或文件名


        path = (struct dir_entry*)kmalloc(sizeof(struct dir_entry),0);
        memset(path,0,sizeof(struct dir_entry));

        path->name = (char*)kmalloc(tmpnamelen+1,0);
        memset(path->name,0,tmpnamelen+1);
        memcpy(path->name,tmpname,tmpnamelen);
        path->name_length = tmpnamelen;

        if(parent->dir_inode->inode_ops->lookup(parent->dir_inode,path) == NULL){
            //log_to_screen(WARNING,"[warning] can't find file or dir:%s",path->name);
            kfree(path->name);
            kfree(path);
            return NULL;
        }

        list_init(&path->child_node);
        list_init(&path->subdirs_list);
        path->parent = parent;
        list_add_to_behind(&parent->subdirs_list,&parent->child_node );

        if(!*name)
            goto last_component;

        while(*name == '/')
            name++;

        if(!*name)
            goto last_slash;

        parent = path;
    }

    last_component:
    last_slash:
    if(flags & 1){      //返回父目录项
        return parent;
    }
    return path;
}