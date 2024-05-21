#include "fat32.h"
#include <VFS.h>
#include <memory.h>
#include <lib.h>
#include <disk.h>
#include <string.h>
#include <time.h>
#include <log.h>
#include <list.h>


struct Disk_Partition_Table DPT;        //硬盘分区表

struct FAT32_BootSector fat32_bootsector;   //FAT32文件系统的启动扇区

struct FAT32_FSInfo fat32_fsinfo;       //辅助扇区


unsigned long FirstDataSector = 0;  //数据区起始扇区
unsigned long BytesPerClus = 0;     //每簇字节数
unsigned long FirstFAT1Sector = 0;  //FAT1表起始扇区
unsigned long FirstFAT2Sector = 0;  //FAT2表起始扇区



struct file_system_type FAT32_fs_type = {
    .name = "FAT32",
    .fs_flags = 0,
    .read_superblock = fat32_read_superblock,
    .next = NULL
};



void Disk1_FAT32_FS_init()
{
    unsigned char buf[512];
    struct FAT32_Directory* dentry = NULL;

    /*  读取引导扇区  */
    memset(buf,0,512);
    IDE_device_operation.transfer(ATA_READ,0,1,buf);

    DPT = *(struct Disk_Partition_Table*)buf;

    print_DPTE_info(DPT.DPTE[0]);
    print_DPTE_info(DPT.DPTE[1]);
    print_DPTE_info(DPT.DPTE[2]);
    print_DPTE_info(DPT.DPTE[3]);


    /*  读取第一个分区的引导扇区  */
    memset(buf,0,512);
    IDE_device_operation.transfer(ATA_READ,DPT.DPTE[0].start_LBA,1,buf);

    fat32_bootsector = *(struct FAT32_BootSector*)buf;

    color_printk(GREEN,BLACK ,"The sector of FSInfo:%d\n",fat32_bootsector.BPB_FSInfo + DPT.DPTE[0].start_LBA );


    /*  获取FSInfo  */
    memset(buf,0,512);
    IDE_device_operation.transfer(ATA_READ,fat32_bootsector.BPB_FSInfo + DPT.DPTE[0].start_LBA,1,buf);
    fat32_fsinfo = *(struct FAT32_FSInfo*)buf;

    /*  判断FSInfo是否正确获取到(记号不对则说明这不是FSInfo)  */
    if(fat32_fsinfo.FSI_LeadSig != 0x41615252 || fat32_fsinfo.FSI_StruSig != 0x61417272){
        log_to_screen(ERROR,"Get Error FSInfo!!!");
    }

    color_printk(GREEN,BLACK ,"freecount:%#x,sig:%#x\n",fat32_fsinfo.FSI_Free_Count );

    FirstDataSector = DPT.DPTE[0].start_LBA + fat32_bootsector.BPB_RsvdSecCnt + fat32_bootsector.BPB_FATSz32 * fat32_bootsector.BPB_NumFATs;

    FirstFAT1Sector = DPT.DPTE[0].start_LBA + fat32_bootsector.BPB_RsvdSecCnt;
    FirstFAT2Sector = FirstFAT1Sector + fat32_bootsector.BPB_FATSz32;
    BytesPerClus = fat32_bootsector.BPB_SecPerClus *fat32_bootsector.BPB_BytesPerSec;

    color_printk(GREEN,BLACK ,"FirstDataSector:%d,FirstFAT1Sector:%d,FirstFAT2Sector:%d",FirstDataSector,FirstFAT1Sector,FirstFAT2Sector );

    dentry = path_walk("/cpu.c",0);
    if(dentry != NULL){
        log_to_screen(INFO,"OK ");
        color_printk(GREEN,BLACK,"file size = %d\n",dentry->Dir_FileSize);
        color_printk(GREEN,BLACK,"clusterHI:%#x,cluster_LO:%#x\n",dentry->Dir_FatClusHI,dentry->Dir_FatClusLO);
        //color_printk(GREEN,BLACK ,"find hello.c\nDir_FstClusHI:%#x,nDir_FstClusLO:%#x,Dir_FileSize:%#x\n",dentry->Dir_FatClusHI,dentry->Dir_FatClusLO,dentry->Dir_FileSize );
    }else{
        log_to_screen(WARNING,"Can't find file.");
    }
    sti();
}



struct FAT32_Directory* path_walk(char* name,unsigned long flags)
{
    char* tmpname = NULL;
    int tmpnamelen = 0;
    struct FAT32_Directory* parent = NULL;
    struct FAT32_Directory* path = NULL;
    char* dentryname = NULL;

    while(*name == '/'){
        name++;
    }
    if(!*name) return NULL;

    parent = (struct FAT32_Directory*)kmalloc(sizeof(struct FAT32_Directory),0 );
    memset(parent,0,sizeof(struct FAT32_Directory));

    dentryname = (char*)kmalloc(PAGE_4K_SIZE,0 );
    memset(dentryname,0,PAGE_4K_SIZE);

    /*  初始的父目录就是第一个目录根目录  */
    parent->Dir_FatClusLO = fat32_bootsector.BPB_RootClus & 0xffff;
    parent->Dir_FatClusHI = (fat32_bootsector.BPB_RootClus >> 16) & 0xfff;

    log_to_screen(INFO,"parent->Dir_FatClusHI:%#x,parent->Dir_FatClusLO:%#x",parent->Dir_FatClusHI,parent->Dir_FatClusLO);

    for(;;){
        tmpname = name;
        while(*name && (*name != '/')) name++;
        tmpnamelen = name - tmpname;        //确定目录名或文件名
        memcpy(dentryname,tmpname ,tmpnamelen );
        dentryname[tmpnamelen] = '\0';

        path = lookup(dentryname,tmpnamelen ,parent ,flags );

        if(path == NULL){   //搜索失败
            log_to_screen(WARNING,"[warning] can't find file or dir:%s",dentryname);
            kfree(dentryname);
            kfree(parent);
            return NULL;
        }
        if(!*name) goto last_component;

        while(*name == '/') name++;

        if(!*name) goto last_slash;

        *parent = *path;
        kfree(path);
    }

    last_component:
    last_slash:
        if(flags & 1){      //返回父目录项
            kfree(dentryname);
            kfree(path);
            return parent;
        }
    kfree(dentryname);
    kfree(parent);
    return path;
}




/*  从指定目录里搜索与目标名匹配的目录项  */
struct FAT32_Directory* lookup(char* name,int namelen,struct FAT32_Directory* dentry,int flags)
{
    unsigned int cluster = 0;
    unsigned long sector = 0;
    unsigned char* buf = kmalloc(BytesPerClus,0 );
    struct FAT32_Directory* p = NULL;
    struct FAT32_Directory* tmpdentry = NULL;
    struct FAT32_LongDirectory* tmpldentry = NULL;
    int j;  //用于存储名称的索引

    cluster = (dentry->Dir_FatClusHI << 16 | dentry->Dir_FatClusLO) & 0xffffffff;
next_cluster:

    /*  通过起始簇号找到对应的扇区  */
    sector = FirstDataSector + (cluster - 2) * fat32_bootsector.BPB_SecPerClus;
    color_printk(GREEN,BLACK ,"lookup cluster:%#x,sector:%#x\n",cluster,sector );

    if(!IDE_device_operation.transfer(ATA_READ,sector,fat32_bootsector.BPB_SecPerClus,buf))
    {
        log_to_screen(ERROR,"[error] FAT32 FS(lookup) read disk ERROR!\n");
        kfree(buf);
        return NULL;
    }

    tmpdentry = (struct FAT32_Directory*)buf;

    for(int i=0;i<BytesPerClus;i+=32,tmpdentry++){    //遍历这个簇的每一个目录项
        if(tmpdentry->Dir_Attr == ATTR_LONG_NAME){
            continue;   //如果是长文件名目录项
        }

        if(tmpdentry->Dir_Name[0] == 0xe5 || tmpdentry->Dir_Name[0] == 0x05 || tmpdentry->Dir_Name[0] == 0x00){ //无效的目录项
            continue;
        }

        /*  筛选出来的就是有效的短目录项  */
        tmpldentry = (struct FAT32_LongDirectory*)tmpdentry - 1;
        j = 0;
        while(tmpldentry->LDIR_Attr == ATTR_LONG_NAME && tmpldentry->LDIR_Ord != 0xe5){
            for(int x=0;x<5;x++){   //匹配文件名的第一部分
                if(j > namelen && tmpldentry->LDIR_Name1[x] == 0xffff){ //0xffff为剩余字符串的填充
                    color_printk(BLUE,BLACK,"%c\n",name[j]);
                    continue;
                }else if(j > namelen || tmpldentry->LDIR_Name1[x] != (unsigned short)(name[j++])){      //出现某个字符不匹配,说明该目录项不是要找的文件的目录项
                    log_to_screen(WARNING,"compare file name fail!");
                    goto continue_cmp_fail;
                }
            }
            for(int x=0;x<6;x++){
                if(j > namelen && tmpldentry->LDIR_Name2[x] == 0xffff){ //0xffff为剩余字符串的填充
                    continue;
                }else if(j > namelen || tmpldentry->LDIR_Name2[x] != (unsigned short)(name[j++])){      //出现某个字符不匹配,说明该目录项不是要找的文件的目录项
                    log_to_screen(WARNING,"compare file name fail!");
                    goto continue_cmp_fail;
                }
            }
            for(int x=0;x<2;x++){
                if(j > namelen && tmpldentry->LDIR_Name3[x] == 0xffff){ //0xffff为剩余字符串的填充
                    continue;
                }else if(j > namelen || tmpldentry->LDIR_Name3[x] != (unsigned short)(name[j++])){      //出现某个字符不匹配,说明该目录项不是要找的文件的目录项
                    log_to_screen(WARNING,"compare file name fail!");
                    goto continue_cmp_fail;
                }
            }
            if(j >= namelen){
                p = (struct FAT32_Directory*)kmalloc(sizeof(struct FAT32_Directory),0 );
                *p = *tmpdentry;
                kfree(buf);
                return p;
            }
            tmpldentry--;
        }

        j = 0;
        color_printk(RED,BLACK,"short dir\n");
        /*  长目录项不一定匹配成功,如果失败了,那么就去匹配短目录项  */
        for(int x=0;x<8;x++){
            switch (tmpdentry->Dir_Name[x]) {
                case ' ':
                    if(!(tmpdentry->Dir_Attr & ATTR_DIRECTORY)){
                        if(name[j] == '.') continue;    //
                        else if(name[j] == tmpdentry->Dir_Name[x]){ //文件名就是带有空格
                            j++;
                            break;
                        }
                        else{
                            //log_to_screen(WARNING,"compare file name fail!");
                            goto continue_cmp_fail;    //匹配失败
                        }
                    }else {
                        if(j < namelen && tmpdentry->Dir_Name[x] == name[j]){
                            j++;
                            break;
                        }else if(j == namelen){
                            continue;
                        }else{
                            //log_to_screen(WARNING,"compare file name fail!");
                            goto continue_cmp_fail;
                        }
                    }
                case 'A' ... 'Z':
                case 'a' ... 'a':
                    if(tmpdentry->Dir_NTRes & LOWERCASE_BASE){  //与Windows兼容
                        if(j < namelen && tmpdentry->Dir_Name[x] + 32 == name[j]){
                            j++;
                            break;
                        }else{
                            //log_to_screen(WARNING,"compare file name fail!");
                            goto continue_cmp_fail;
                        }
                    }else{
                        if(j < namelen && tmpdentry->Dir_Name[x] == name[j]){
                            j++;
                            break;
                        }else{
                            //log_to_screen(WARNING,"compare file name fail!");
                            goto continue_cmp_fail;
                        }
                    }
                    break;
                case 0 ... 9:
                    if(j < namelen && tmpdentry->Dir_Name[x] == name[j]){
                        j++;
                        break;
                    }else{
                        log_to_screen(WARNING,"compare file name fail!");
                        goto continue_cmp_fail;
                    }
                default:
                    j++;
                    break;
            }
        }
        /*  匹配扩展名  */
        if(!(tmpdentry->Dir_Attr & ATTR_DIRECTORY)){
            j++;
            for(int x=8;x<11;x++){
                switch (tmpdentry->Dir_Name[x]) {
                    case 'a' ... 'z':
                    case 'A' ... 'Z':
                        if(tmpdentry->Dir_NTRes & LOWERCASE_BASE){
                            if(tmpdentry->Dir_Name[x] + 32 == name[j]){
                                j++;
                                break;
                            }else{
                                log_to_screen(WARNING,"compare file name fail!");
                                goto continue_cmp_fail;
                            }
                        }else{
                            if(tmpdentry->Dir_Name[x] == name[j]){
                                j++;
                                break;
                            }else{
                                log_to_screen(WARNING,"compare file name fail!");
                                goto continue_cmp_fail;
                            }
                        }

                    case '0' ... '9':
                    case ' ':
                        if(tmpdentry->Dir_Name[x] == name[j]){
                            j++;
                            break;
                        }else{
                            log_to_screen(WARNING,"compare file name fail!");
                            goto continue_cmp_fail;
                        }
                    default:
                        log_to_screen(WARNING,"compare file name fail!");
                        goto continue_cmp_fail;
                }
            }

        }

        p = (struct FAT32_Directory*)kmalloc(sizeof(struct FAT32_Directory),0 );
        *p = *tmpdentry;
        kfree(buf);
        return p;


        continue_cmp_fail:;     //匹配失败了
    }

    /*
     *如果搜索完整个簇之后还没有找到对应文件名的目录项
     * 那么就要去父目录的下一个簇号(一个目录并不是只有一个簇号)
     * 在父目录的下一个簇里搜索目标文件名对应的目录项
     * 如此往复,直到搜索到父目录的最后一个簇为止
     */

    cluster = DISK1_FAT32_read_FAT_Entry(cluster);
    if(cluster < 0xffffff7){    //进行下一个簇的匹配(大于这个簇号则不需要再匹配)
        goto next_cluster;
    }
    kfree(buf);
    return NULL;
}



/*  读取FAT指定表项的值  */
unsigned int DISK1_FAT32_read_FAT_Entry(unsigned int fat_entry)
{
    unsigned int buf[128];      //每扇区有128个FAT表项(4B)
    memset(buf,0,512);
    IDE_device_operation.transfer(ATA_READ,FirstFAT1Sector + (fat_entry >> 7),1,(unsigned char*)buf);   //fat_entry >> 7是该目录项所在扇区索引
    log_to_screen(INFO,"fat_entry:%#lx,next fat_entry:%#lx",fat_entry,buf[fat_entry & 0x7f]);
    return buf[fat_entry & 0x7f] & 0xffffffff;  //查询到fat_entry这个表项的值
}



/*  把值写入到指定FAT表项中  */
unsigned long DISK1_FAT32_write_FAT_Entry(unsigned int fat_entry,unsigned int value)
{
    unsigned int buf[128];
    memset(buf,0,512);
    IDE_device_operation.transfer(ATA_READ,FirstFAT1Sector + (fat_entry >> 7),1,(unsigned char*)buf);
    /*  先清空表项原来的值再写入新值  */
    buf[fat_entry & 0x7f] = (buf[fat_entry & 0x7f] & 0xf0000000) | (value & 0xffffffff);

    /*  更新两个FAT表  */
    IDE_device_operation.transfer(ATA_WRITE,FirstFAT1Sector,0,(unsigned char*)buf);
    IDE_device_operation.transfer(ATA_WRITE,FirstFAT2Sector,0,(unsigned char*)buf);
    return 1;
}




void fat32_put_superblock(struct super_block* sb)
{
    kfree(sb->private_sb_info);
    kfree(sb->root->dir_inode->private_index_info);
    kfree(sb->root->dir_inode);
    kfree(sb->root);
    kfree(sb);
}



/*  将修改后的inode结构写回到硬盘扇区中(inode携带新信息)  */
void fat32_write_inode(struct index_node* inode)
{
    struct FAT32_Directory* fdentry = NULL;
    struct FAT32_Directory* buf = NULL;
    struct FAT32_inode_info* finode = inode->private_index_info;
    struct FAT32_sb_info* fsbi = inode->sb->private_sb_info;
    unsigned long sector = 0;

    if(finode->dentry_location == 0){
        log_to_screen(WARNING,"FS ERROR:write root inode!");
        return;
    }
    sector = fsbi->Data_firstsector = (finode->dentry_location - 2) *fsbi->sector_per_cluster;

    buf = (struct FAT32_Directory *)kmalloc(fsbi->bytes_per_cluster,0);
    memset(buf,0,fsbi->bytes_per_cluster);

    IDE_device_operation.transfer(ATA_READ,sector,fsbi->bytes_per_cluster,(unsigned char*)buf);
    fdentry = buf + finode->dentry_position;    //刚读出来的待修改的数据

    /*  对旧数据进行修改  */
    fdentry->Dir_FileSize = inode->file_size;
    fdentry->Dir_FatClusLO = (finode->first_cluster & 0xffff);
    fdentry->Dir_FatClusHI = (fdentry->Dir_FatClusHI & 0xf000) | (finode->first_cluster >> 16);


    IDE_device_operation.transfer(ATA_WRITE,sector,fsbi->sector_per_cluster,(unsigned char*)buf);
    kfree(buf);
}





/*  将修改后的superblock结构写回到硬盘中  */
void fat32_write_superblock(struct super_block* sb)
{

}



long FAT32_compare(struct dir_entry* parent_dentry,char* source_filename,char* destination_filename)
{

}



long FAT32_hash(struct dir_entry* parent_dentry,char* filename)
{

}



long FAT32_release(struct dir_entry* dentry)
{

}


long FAT32_iput(struct dir_entry* dentry,struct index_node* inode)
{

}




long FAT32_create(struct index_node* inode,struct dir_entry* dentry)
{

}



struct dir_entry* FAT32_lookup(struct index_node* parent_inode,struct dir_entry* destination_dentry)
{

}


long FAT32_mkdir(struct index_node* inode,struct dir_entry* dentry,int mode)
{

}


long FAT32_rmdir(struct index_node* inode,struct dir_entry* dentry)
{

}



long FAT32_rename(struct index_node* old_inode,struct dir_entry* old_dentry,struct index_node* new_inode,struct dir_entry* new_dentry)
{

}


long FAT32_getattr(struct dir_entry* dentry,unsigned long* attr)
{

}


long FAT32_setattr(struct dir_entry* dentry,unsigned long* attr)
{

}






long FAT32_open(struct index_node* inode,struct file* filep){
    return 1;
}



long FAT32_close(struct index_node* inode,struct file* filep){
    return 1;
}


long FAT32_read(struct file* filep,unsigned char* buf,unsigned long count,long * position)
{
    return 1;
}


long FAT32_write(struct file* filep,unsigned char* buf,unsigned long count,long* position)
{
    return 1;
}


long FAT32_lseek(struct file* filep,long offset,long origin){
    return 1;
}



long FAT32_ioctl(struct index_node* inode,struct file* filep,unsigned long cmd,unsigned long arg)
{
    return 1;
}




struct super_block_operations FAT32_sb_ops = {
    .put_superblock = fat32_put_superblock,
    .write_superblock = fat32_write_superblock,
    .write_inode = fat32_write_inode,
};



struct dir_entry_options FAT32_dentry_ops = {
    .compare = FAT32_compare,
    .hash = FAT32_hash,
    .release = FAT32_release,
    .iput = FAT32_iput
};



struct index_node_operations FAT32_inode_ops = {
    .create = FAT32_create,
    .lookup = FAT32_lookup,
    .mkdir = FAT32_mkdir,
    .getattr = FAT32_getattr,
    .setattr = FAT32_setattr,
    .rename = FAT32_rename
};



struct file_operations FAT32_file_ops = {
    .open = FAT32_open,
    .close = FAT32_close,
    .read = FAT32_read,
    .write = FAT32_write,
    .lseek = FAT32_lseek,
    .ioctl = FAT32_ioctl
};



/*  从FAT32中获取信息并构造超级块  */
struct super_block* fat32_read_superblock(struct Disk_Partition_Table_Entry* DPTE,void* buf)
{
    struct super_block* sbp = NULL;         //超级块
    struct FAT32_inode_info* finode = NULL;
    struct FAT32_BootSector* fbs = NULL;    //引导扇区
    struct FAT32_sb_info* fsbi = NULL;


    /*  fat32超级块  */
    sbp = (struct super_block*)kmalloc(sizeof(struct super_block),0);
    memset(sbp,0,sizeof(struct super_block));

    sbp->sb_ops = &FAT32_sb_ops;
    sbp->private_sb_info = (struct FAT32_sb_info*)kmalloc(sizeof(struct FAT32_sb_info),0);
    memset(sbp->private_sb_info,0,sizeof(struct FAT32_sb_info));


    /*  fat32引导扇区  */
    fbs = (struct FAT32_BootSector*)buf;
    fsbi = sbp->private_sb_info;
    fsbi->start_sector = DPTE->start_sector;
    fsbi->sector_count = DPTE->sectors_limit;
    fsbi->sector_per_cluster = fbs->BPB_SecPerClus;
    fsbi->bytes_per_cluster = fbs->BPB_SecPerClus * fbs->BPB_BytesPerSec;
    fsbi->bytes_per_sector = fbs->BPB_BytesPerSec;

    /*
     * 数据区的第一个扇区
     * 第一个分区的起始扇区 保留扇区 FAT表1 FAT表2 数据区
     */
    fsbi->Data_firstsector = DPTE->start_sector + fbs->BPB_RsvdSecCnt + fbs->BPB_FATSz32 * fbs->BPB_NumFATs;
    fsbi->FAT1_firstsector = DPTE->start_sector + fbs->BPB_RsvdSecCnt;
    fsbi->sector_per_FAT = fbs->BPB_FATSz32;
    fsbi->NumFATs = fbs->BPB_NumFATs;
    fsbi->fsinfo_sector_infat = fbs->BPB_FSInfo;
    fsbi->bootsector_bk_infat = fbs->BPB_BKBootSec;

    fsbi->fat_fsinfo = (struct FAT32_FSInfo*)kmalloc(sizeof(struct FAT32_FSInfo),0);
    memset(fsbi->fat_fsinfo,0,512);
    IDE_device_operation.transfer(ATA_READ,DPTE->start_LBA + fbs->BPB_FSInfo,1,(unsigned char*)fsbi->fat_fsinfo);


    /*  目录项  */
    sbp->root = (struct dir_entry*)kmalloc(sizeof(struct dir_entry),0);
    memset(sbp->root,0,sizeof(struct dir_entry));

    list_init(&sbp->root->child_node);
    list_init(&sbp->root->subdirs_list);
    sbp->root->parent = sbp->root;
    sbp->root->dir_ops = &FAT32_dentry_ops;
    sbp->root->name = (char*)kmalloc(2,0 );
    sbp->root->name[0] = '/';
    sbp->root->name_length = 1;



    /*  index node  */
    sbp->root->dir_inode = (struct index_node*)kmalloc(sizeof(struct index_node),0);
    memset(sbp->root->dir_inode,0,sizeof(struct index_node));
    sbp->root->dir_inode->inode_ops = &FAT32_inode_ops;
    sbp->root->dir_inode->f_ops = &FAT32_file_ops;
    sbp->root->dir_inode->file_size = 0;
    sbp->root->dir_inode->blocks = (sbp->root->dir_inode->file_size + fsbi->bytes_per_cluster - 1) / fsbi->bytes_per_cluster;  //该文件需要占用多少个簇(向上取整)
    sbp->root->dir_inode->attribute = FS_ATTR_DIR;
    sbp->root->dir_inode->sb = sbp;




    /*  fat32 root inode  */
    sbp->root->dir_inode->private_index_info = (struct FAT32_inode_info*)kmalloc(sizeof(struct FAT32_inode_info),0);
    memset(sbp->root->dir_inode->private_index_info,0,sizeof(struct FAT32_inode_info));
    finode = (struct FAT32_inode_info*)sbp->root->dir_inode->private_index_info;
    finode->first_cluster = fbs->BPB_RootClus;
    finode->dentry_location = 0;
    finode->dentry_position = 0;
    finode->create_date = 0;
    finode->create_time = 0;
    finode->write_date = 0;
    finode->write_time = 0;

    return sbp;
}





