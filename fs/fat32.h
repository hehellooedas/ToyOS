#ifndef __FS_FAT32_H
#define __FS_FAT32_H


#include <printk.h>


/*  硬盘分区表项(16B)  */
struct Disk_Partition_Table_Entry{
    unsigned char flags;
    unsigned char start_head;
    unsigned short
                    start_sector:6,
                    start_cylinder:10;
    unsigned char type;
    unsigned char end_head;
    unsigned short
                    end_sector:6,
                    end_cylinder:10;
    unsigned int start_LBA;
    unsigned int sectors_limit;
}__attribute__((packed));




/*  硬盘分区表  */
struct Disk_Partition_Table{
    unsigned char BS_Reserved[446];     //主引导记录MBR
    struct Disk_Partition_Table_Entry DPTE[4];   //4个分区表项
    unsigned short BS_TrailSig;         //0x55aa标记
}__attribute__((packed));




/*  引导扇区(MBR)  */
struct FAT32_BootSector{
    unsigned char BS_jmpBoot[3];        //跳转指令
    unsigned char BS_OEMName[8];        //OEM厂商名
    unsigned short BPB_BytesPerSec;     //每扇区字节数
    unsigned char BPB_SecPerClus;       //每簇扇区数
    unsigned short BPB_RsvdSecCnt;      //保留扇区数
    unsigned char BPB_NumFATs;          //FAT表的份数
    unsigned short BPB_RootEntCnt;      //根目录可容纳的目录项数
    unsigned short BPB_TotSec16;        //总扇区数
    unsigned char BPB_Media;            //介质描述符
    unsigned short BPB_FATSz16;         //为0
    unsigned short BPB_SecPerTrk;       //每磁道扇区数
    unsigned short BPB_NumHeads;        //磁头数
    unsigned int BPB_hiddSec;           //隐藏扇区数
    unsigned int BPB_TotSec32;          //总扇区数(若BPB_TotSec16=0,则由这个变量记录)

    unsigned int BPB_FATSz32;           //每FAT扇区数
    unsigned short BPB_ExtFlags;        //拓展标志
    unsigned short BPB_FSVer;           //FAT32文件系统的版本号
    unsigned int BPB_RootClus;          //根目录起始簇号(位于数据区的起始簇中)
    unsigned short BPB_FSInfo;          //FSInfo结构体所在的扇区号
    unsigned short BPB_BKBootSec;       //引导扇区的备份扇区号
    unsigned char BPB_Reserved[12];     //保留使用

    unsigned char BS_DrvNum;            //int 0x13的驱动器号
    unsigned char BS_Reserved1;         //不使用
    unsigned char BS_BootSig;           //拓展引导标记
    unsigned int BS_VolID;              //卷序列号
    unsigned char BS_VolLab[11];        //卷标
    unsigned char BS_FileSysType[8];    //文件系统类型

    unsigned char BootCode[420];        //引导代码
    unsigned short BS_TrailSig;         //0x55aa标记
}__attribute__((packed));




/*
 * 在FAT32文件系统的(保留区域)里加入的辅助性的扇区结构FSInfo
 * 参考值不是实时更新的准确数值,只是辅助计算和索引空闲簇
 * 当参考值为0xffffffff 则必须重新计算
 * 占用一整个扇区,但大多数空间不使用
 */
struct FAT32_FSInfo{
    unsigned int FSI_LeadSig;           //标识符 0x41615252
    unsigned char FSI_Resevered1[480];  //保留
    unsigned int FSI_StruSig;           //标识符 0x61417272
    unsigned int FSI_Free_Count;        //上一次记录的空闲簇数量(参考值)
    unsigned int FSI_Nxt_Free;          //空闲簇的起始搜索位置(参考值)
    unsigned char FSI_Resevered2[12];
    unsigned int FSI_TrailSig;          //结束标志 0xaa550000
}__attribute__((packed));




/*  目录项属性  */
#define ATTR_READ_ONLY      (1 << 0)    //只读
#define ATTR_HIDDEN         (1 << 1)    //隐藏
#define ATTR_SYSTEM         (1 << 2)    //系统文件
#define ATTR_VOLUME_ID      (1 << 3)    //卷标
#define ATTR_DIRECTORY      (1 << 4)    //目录
#define ATTR_ARCHIVE        (1 << 5)    //存档
#define ATTR_LONG_NAME (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID) //长文件名



/*  FAT32目录项(32B)  */
struct FAT32_Directory{
    unsigned char Dir_Name[11];     //目录名(文件名最多8,扩展名最多3,不足使用空格补充)
    unsigned char Dir_Attr;         //文件属性(短目录项和长目录项的Attr在同一个地方)
    unsigned char Dir_NTRes;        //保留
    unsigned char Dir_CrtTimeTenth; //文件创建时的时间戳
    unsigned short Dir_CrtTime;     //文件创建的时间
    unsigned short Dir_CrtDate;     //文件创建的日期
    unsigned short Dir_LastAccDate; //最后访问日期
    unsigned short Dir_FatClusHI;   //起始簇号(高位)
    unsigned short Dir_WrtTime;     //最后写入时间
    unsigned short Dir_WrtDate;     //最后写入日期
    unsigned short Dir_FatClusLO;   //起始簇号(低位)
    unsigned int Dir_FileSize;      //文件大小
}__attribute__((packed));




/*  长目录项(32B)  */
struct FAT32_LongDirectory{
    unsigned char LDIR_Ord;         //长目录项的序号
    unsigned short LDIR_Name1[5];   //长文件名的1-5个字符(名称的第一部分)
    unsigned char LDIR_Attr;        //属性必须为ATTR_LONG_NAME
    unsigned char LDIR_Type;        //如果为0,说明是长目录项的子项
    unsigned char LDIR_Chksum;      //短文件名的校验和
    unsigned short LDIR_Name2[6];   //6-11(第二部分)
    unsigned short LDIR_FstClusLO;  //必须为0
    unsigned short LDIR_Name3[2];   //12-13(第三部分)
}__attribute__((packed));


#define LOWERCASE_BASE  (8)
#define LOWERCASE_EXT   (16)


void Disk1_FAT32_FS_init();
struct FAT32_Directory* path_walk(char* name,unsigned long flags);
struct FAT32_Directory* lookup(char* name,int namelen,struct FAT32_Directory* dentry,int flags);
unsigned int DISK1_FAT32_read_FAT_Entry(unsigned int fat_entry);
unsigned long DISK1_FAT32_write_FAT_Entry(unsigned int fat_entry,unsigned int value);



/*  打印硬盘分区表信息  */
static __attribute__((always_inline))
void print_DPTE_info(struct Disk_Partition_Table_Entry T)
{
    color_printk(GREEN,BLACK ,"flags:%d,type:%d,start_head:%d,\
start_sector:%d,start_cylinder:%d,end_head:%d,end_sector:%d,\
end_cylinder:%d,start_LBA:%d,sectors_limit:%d\n",T.flags,T.type,T.start_head,T.start_sector,T.start_cylinder,\
T.end_head,T.end_sector,T.end_cylinder,T.start_LBA,T.sectors_limit );
}




/*  FAT32文件系统特有的超级块信息  */
struct FAT32_sb_info{
    unsigned long start_sector; //起始扇区
    unsigned long sector_count;

    long sector_per_cluster;    //每簇扇区数
    long bytes_per_cluster;     //每簇字节数
    long bytes_per_sector;      //每扇区字节数

    unsigned long Data_firstsector;     //数据区起始扇区
    unsigned long FAT1_firstsector;     //fat1表起始扇区
    unsigned long sector_per_FAT;       //每个FAT表占用几个扇区
    unsigned long NumFATs;

    unsigned long fsinfo_sector_infat;
    unsigned long bootsector_bk_infat;

    struct FAT32_FSInfo* fat_fsinfo;
};



/*  FAT32特有的inode信息  */
struct FAT32_inode_info{
    unsigned long first_cluster;
    unsigned long dentry_location;
    unsigned long dentry_position;

    unsigned short create_date;
    unsigned short create_time;
    unsigned short write_date;
    unsigned short write_time;

};


struct super_block* fat32_read_superblock(struct Disk_Partition_Table_Entry* DPTE,void* buf);


#endif