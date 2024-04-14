#include "fat32.h"
#include <memory.h>
#include <lib.h>
#include <disk.h>
#include <string.h>
#include <printk.h>


struct Disk_Partition_Table DPT;

struct FAT32_BootSector fat32_bootsector;

struct FAT32_FSInfo fat32_fsinfo;


unsigned long FirstDataSector = 0;  //数据区起始扇区
unsigned long BytesPerClus = 0;     //每簇字节数
unsigned long FirstFAT1Sector = 0;  //FAT1表起始扇区
unsigned long FirstFAT2Sector = 0;


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

    dentry = path_walk("/hello.c",0);
}



struct FAT32_Directory* path_walk(char* name,unsigned long flags)
{
    struct FAT32_Directory* path = NULL;



    return path;
}


