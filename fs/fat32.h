#ifndef __FS_FAT32_H
#define __FS_FAT32_H



/*  硬盘分区表项  */
struct Disk_Patition_Table_Entry{
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
    struct Disk_Patition_Table_Entry DPTE[4];
    unsigned short BS_TrailSig;         //0x55aa标记
}__attribute__((packed));




/*  引导扇区  */
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
    unsigned int BPB_TotSec32;        //总扇区数(若BPB_TotSec16=0,则由这个变量记录)

    unsigned int BPB_FATSz32;           //每FAT扇区数
    unsigned short BPB_ExtFlags;        //拓展标志
    unsigned short BPB_FSVer;           //FAT32文件系统的版本号
    unsigned int BPB_RootClus;          //根目录起始簇号
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
 * 在FAT32文件系统的保留区域里加入的辅助性的扇区结构FSInfo
 * 参考值不是实时更新的准确数值,只是辅助计算和索引空闲簇
 * 当参考值为0xffffffff 则必须重新计算
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
    unsigned char Dir_Name[11];     //目录名
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



void Disk1_FAT32_FS_init();
struct FAT32_Directory* path_walk(char* name,unsigned long flags);
struct FAT32_Directory* lookup(char* name,int namelen,struct FAT32_Directory* dentry,int flags);
unsigned int DISK1_FAT32_read_FAT_Entry(unsigned int fat_entry);
unsigned int DISK1_FAT32_write_FAT_Entry(unsigned int fat_entry,unsigned int value);


#endif