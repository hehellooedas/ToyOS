#ifndef __FS_VFS_H
#define __FS_VFS_H

#include <list.h>


struct dir_entry{
    char* name;
    int name_length;
    struct List child_node;
    struct List subdirs_list;

    struct index_node* dir_inode;
    struct dir_entry* parent;
    struct dir_entry_options* dir_ops;
};



struct super_block{
    struct dir_entry* root;
    struct super_block_optiopns* sb_ops;
    void* private_sb_info;
};



#endif