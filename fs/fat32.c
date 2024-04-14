#include "fat32.h"
#include "memory.h"
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
    for(int i=0;i<512;i++){
        color_printk(GREEN,BLACK ,"%#x ",buf[i] );
    }
    //IDE_device_operation.transfer(ATA_READ,DPT.DPTE[0].start_LBA,1,buf);
}



struct FAT32_Directory* path_walk(char* name,unsigned long flags)
{
    struct FAT32_Directory* path = NULL;



    return path;
}


