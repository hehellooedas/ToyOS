#ifndef __FS_VFS_H
#define __FS_VFS_H

#include <fat32.h>
#include <list.h>



/*
 * VFS虚拟文件系统是一个抽象层,对用户提供一组固定的API
 * 对应用程序而言文件系统变得透明,用户程序无需关心底层到底是哪一种文件系统
 */


#define FS_ATTR_FILE    (1UL << 0)
#define FS_ATTR_DIR     (1UL << 1)


/*
 * dentry结构用于描述文件/目录在文件系统中的层级关系
 * 代表一个目录项,是路径的一个组成部分
 */
struct dir_entry{
    char* name;
    int name_length;
    struct List child_node;
    struct List subdirs_list;

    struct index_node* dir_inode;
    struct dir_entry* parent;
    struct dir_entry_options* dir_ops;
};



struct dir_entry_options{
    long (*compare)(struct dir_entry* parent_dentry,char* source_filename,char* destination_filename);
    long (*hash)(struct dir_entry* parent_dentry,char* filename);
    long (*release)(struct dir_entry* dentry);
    long (*iput)(struct dir_entry* dentry,struct index_node* inode);
};





/*
 * 超级块记录着目标文件系统的引导扇区信息
 * 还有OS为文件系统分配的资源信息
 * 代表了一个具体的已安装的文件系统
 */
struct super_block{
    struct dir_entry* root;     //为了方便搜索而抽象出来的
    struct super_block_operations* sb_ops;
    void* private_sb_info;      //保存各类文件系统特有的数据结构
};


struct super_block_operations{
    void (*write_superblock)(struct super_block* sb);
    void (*put_superblock)(struct super_block* sb);
    void (*write_inode)(struct index_node* inode);
};





/*
 * inode节点是VFS的核心
 * inode记录着文件在文件系统中的物理信息和文件在OS的抽象信息
 * 代表了一个具体文件/目录
 */
struct index_node{
    unsigned long file_size;    //文件大小
    unsigned long blocks;       //
    unsigned long attribute;    //用自己的方式描述文件的属性

    struct super_block* sb;
    struct file_operations* f_ops;
    struct index_node_operations* inode_ops;
    void* private_index_info;   //文件系统特有的inode信息
};


struct index_node_operations{
    long (*create)(struct index_node* inode,struct dir_entry* dentry);
    struct dir_entry* (*lookup)(struct index_node* parent_inode,struct dir_entry* destination_dentry);
    long (*mkdir)(struct index_node* inode,struct dir_entry* dentry,int mode);
    long (*rmdir)(struct index_node* inode,struct dir_entry* dentry);
    long (*rename)(struct index_node* old_inode,struct dir_entry* old_dentry,struct index_node* new_inode,struct dir_entry* new_dentry);
    long (*getattr)(struct dir_entry* dentry,unsigned long* attr);
    long (*setattr)(struct dir_entry* dentry,unsigned long* attr);
};




/*
 * file结构是进程连接VFS的纽带,它是一种抽象结构
 * 代表由进程打开的文件
 */
struct file{
    long position;
    unsigned long mode;

    struct dir_entry* dentry;
    struct file_operations* f_ops;
    void* private_data;
};


/*  文件操作的函数集合(方法)  */
struct file_operations{
    long (*open)(struct index_node* inode,struct file* filep);
    long (*close)(struct index_node* inode,struct file* filep);
    long (*read)(struct file* filep,unsigned char* buf,unsigned long count,long* position);
    long (*write)(struct file* filep,unsigned char* buf,unsigned long count,long* position);
    long (*lseek)(struct file* filep,long offset,long origin);
    long (*ioctl)(struct index_node* inode,struct file* filep,unsigned long cmd,unsigned long arg);
};





/*  每个注册的文件系统都由该结构体来表示  */
struct file_system_type{
    char* name;     //文件系统的名字
    int fs_flags;   //文件系统的类型标志
    struct super_block* (*read_superblock)(struct Disk_Partition_Table_Entry* DPTE,void* buf); //解析文件系统引导扇区的方法
    struct file_system_type* next;
};






/*  函数声明  */
struct super_block* mount_fs(char* name,struct Disk_Partition_Table_Entry* DPTE,void* buf);
unsigned long register_filesystem(struct file_system_type* fs);



#endif