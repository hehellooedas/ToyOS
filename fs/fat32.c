#include "fat32.h"
#include <memory.h>
#include <lib.h>
#include <disk.h>
#include <string.h>
#include <printk.h>
#include <time.h>


struct Disk_Partition_Table DPT;        //硬盘分区表

struct FAT32_BootSector fat32_bootsector;   //FAT32文件系统的启动扇区

struct FAT32_FSInfo fat32_fsinfo;       //辅助扇区


unsigned long FirstDataSector = 0;  //数据区起始扇区
unsigned long BytesPerClus = 0;     //每簇字节数
unsigned long FirstFAT1Sector = 0;  //FAT1表起始扇区
unsigned long FirstFAT2Sector = 0;  //FAT2表起始扇区


void Disk1_FAT32_FS_init()
{
    unsigned char buf[512];
    struct FAT32_Directory* dentry = NULL;

    IDE_device_operation.transfer(ATA_READ,0,1,buf);

    DPT = *(struct Disk_Partition_Table*)buf;

    memset(buf,0,512);
    IDE_device_operation.transfer(ATA_READ,DPT.DPTE[0].start_LBA,1,buf);

    fat32_bootsector = *(struct FAT32_BootSector*)buf;

    color_printk(GREEN,BLACK ,"The sector of FSInfo:%d\n",fat32_bootsector.BPB_FSInfo + DPT.DPTE[0].start_LBA );

    memset(buf,0,512);
    IDE_device_operation.transfer(ATA_READ,1,1,buf);
    fat32_fsinfo = *(struct FAT32_FSInfo*)buf;

    color_printk(GREEN,BLACK ,"freecount:%#x\n",fat32_fsinfo.FSI_Free_Count );

    FirstDataSector = DPT.DPTE[0].start_LBA + fat32_bootsector.BPB_RsvdSecCnt + fat32_bootsector.BPB_FATSz32 * fat32_bootsector.BPB_NumFATs;

    FirstFAT1Sector = DPT.DPTE[0].start_LBA + fat32_bootsector.BPB_RsvdSecCnt;
    FirstFAT2Sector = DPT.DPTE[0].start_LBA + fat32_bootsector.BPB_RsvdSecCnt + fat32_bootsector.BPB_FATSz32;
    BytesPerClus = fat32_bootsector.BPB_SecPerClus *fat32_bootsector.BPB_BytesPerSec;

    dentry = path_walk("/hello.c",0);
    if(dentry != NULL){
        color_printk(GREEN,BLACK ,"find hello.c\nDir_FstClusHI:%#x,nDir_FstClusLO:%#x,Dir_FileSize:%#x\n",dentry->Dir_FatClusHI,dentry->Dir_FatClusLO,dentry->Dir_FileSize );
    }else{
        color_printk(RED,BLACK ,"[warning] Can't find file.\n" );
    }
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

    parent->Dir_FatClusLO = fat32_bootsector.BPB_RootClus & 0xffff;
    parent->Dir_FatClusHI = (fat32_bootsector.BPB_RootClus >> 16) & 0xfff;

    for(;;){
        tmpname = name;
        while(*name && (*name != '/')) name++;
        tmpnamelen = tmpname - name;        //确定目录名或文件名
        memcpy(dentryname,tmpname ,tmpnamelen );
        dentryname[tmpnamelen] = '\0';

        if(path == NULL){   //搜索失败
            color_printk(RED,BLACK ,"[warning] can't find file or dir:%s\n",dentryname );
            kfree(dentryname);
            kfree(parent);
            return NULL;
        }

        *parent = *path;
        kfree(path);
    }
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
    int j;

    cluster = (dentry->Dir_FatClusHI << 16 | dentry->Dir_FatClusLO) & 0xffffffff;
next_cluster:
    sector = FirstDataSector + (cluster - 2) * fat32_bootsector.BPB_SecPerClus;
    color_printk(GREEN,BLACK ,"lookup cluster:%#x,sector:%#x\n",cluster,sector );

    if(!IDE_device_operation.transfer(ATA_READ,sector,fat32_bootsector.BPB_SecPerClus,buf))
    {
        color_printk(GREEN,BLACK ,"[error] FAT32 FS(lookup) read disk ERROR!\n" );
        kfree(buf);
        return NULL;
    }
    tmpdentry = (struct FAT32_Directory*)buf;

    for(int i=0;i<BytesPerClus;i+=32,dentry++){    //遍历这个簇的每一个目录项

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
            for(int x=0;i<5;x++){   //匹配文件名的第一部分
                if(j > namelen && tmpldentry->LDIR_Name1[x] == 0xffff){ //0xffff为剩余字符串的填充
                    continue;
                }else if(j > namelen || tmpldentry->LDIR_Name1[x] != (unsigned short)(name[j++])){      //出现某个字符不匹配,说明该目录项不是要找的文件的目录项
                    goto continue_cmp_fail;
                }
            }
            for(int x=0;x<6;x++){
                if(j > namelen && tmpldentry->LDIR_Name2[x] == 0xffff){ //0xffff为剩余字符串的填充
                    continue;
                }else if(j > namelen || tmpldentry->LDIR_Name2[x] != (unsigned short)(name[j++])){      //出现某个字符不匹配,说明该目录项不是要找的文件的目录项
                    goto continue_cmp_fail;
                }
            }
            for(int x=0;x<2;x++){
                if(j > namelen && tmpldentry->LDIR_Name3[x] == 0xffff){ //0xffff为剩余字符串的填充
                    continue;
                }else if(j > namelen || tmpldentry->LDIR_Name3[x] != (unsigned short)(name[j++])){      //出现某个字符不匹配,说明该目录项不是要找的文件的目录项
                    goto continue_cmp_fail;
                }
            }
            if(j >namelen){
                p = (struct FAT32_Directory*)kmalloc(sizeof(struct FAT32_Directory),0 );
                *p = *tmpdentry;
                kfree(buf);
                return p;
            }
            tmpldentry--;
        }
    }
    return p;
    continue_cmp_fail:;     //匹配失败了
    if(cluster < 0xffffff7){    //进行下一个簇的匹配
        goto next_cluster;
    }
    kfree(buf);
    return NULL;
}



unsigned int DISK1_FAT32_read_FAT_Entry(unsigned int fat_entry)
{
    unsigned int buf[128];      //每扇区有128个FAT表项(4B)
    memset(buf,0,512);
    IDE_device_operation.transfer(ATA_READ,FirstFAT1Sector + (fat_entry >> 7),1,(unsigned char*)buf);
    color_printk(GREEN,BLACK ,"" );
    return buf[fat_entry & 0x70] & 0xffffffff;
}



unsigned int DISK1_FAT32_write_FAT_Entry(unsigned int fat_entry,unsigned int value)
{
    unsigned int buf[128];
    memset(buf,0,512);
    IDE_device_operation.transfer(ATA_READ,FirstFAT1Sector + (fat_entry >> 7),1,(unsigned char*)buf);
    return 1;
}



