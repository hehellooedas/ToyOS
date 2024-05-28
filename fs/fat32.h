#ifndef __FS_FAT32_H
#define __FS_FAT32_H


#include <printk.h>
#include <log.h>



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




/*  主引导记录(从这里获取磁盘分区表)  */
struct Disk_Partition_Table{
    unsigned char BS_Reserved[446];     //存放启动引导程序的区块
    struct Disk_Partition_Table_Entry DPTE[4];   //4个分区表项(记录整个硬盘分区的状态,总共64B)
    unsigned short BS_TrailSig;         //0x55aa魔数标记
}__attribute__((packed));




/*  启动扇区(MBR)  */
struct FAT32_BootSector{
    unsigned char  BS_jmpBoot[3];        //跳转指令
    unsigned char  BS_OEMName[8];        //OEM厂商名
    unsigned short BPB_BytesPerSec;      //每扇区字节数
    unsigned char  BPB_SecPerClus;       //每簇扇区数
    unsigned short BPB_RsvdSecCnt;       //保留扇区数
    unsigned char  BPB_NumFATs;          //FAT表的份数
    unsigned short BPB_RootEntCnt;       //根目录可容纳的目录项数
    unsigned short BPB_TotSec16;         //总扇区数
    unsigned char  BPB_Media;            //介质描述符
    unsigned short BPB_FATSz16;          //为0
    unsigned short BPB_SecPerTrk;        //每磁道扇区数
    unsigned short BPB_NumHeads;         //磁头数
    unsigned int   BPB_hiddSec;          //隐藏扇区数
    unsigned int   BPB_TotSec32;         //总扇区数(若BPB_TotSec16=0,则由这个变量记录)

    unsigned int   BPB_FATSz32;          //每FAT扇区数
    unsigned short BPB_ExtFlags;         //拓展标志
    unsigned short BPB_FSVer;            //FAT32文件系统的版本号
    unsigned int   BPB_RootClus;         //根目录起始簇号(位于数据区的起始簇中)
    unsigned short BPB_FSInfo;           //FSInfo结构体所在的扇区号
    unsigned short BPB_BKBootSec;        //引导扇区的备份扇区号
    unsigned char  BPB_Reserved[12];     //保留使用

    unsigned char  BS_DrvNum;            //int 0x13的驱动器号
    unsigned char  BS_Reserved1;         //不使用
    unsigned char  BS_BootSig;           //拓展引导标记
    unsigned int   BS_VolID;             //卷序列号
    unsigned char  BS_VolLab[11];        //卷标
    unsigned char  BS_FileSysType[8];    //文件系统类型

    unsigned char  BootCode[420];        //引导代码
    unsigned short BS_TrailSig;          //0x55aa标记
}__attribute__((packed));




/*
 * 在FAT32文件系统的(保留区域)里加入的辅助性的扇区结构FSInfo
 * 参考值不是实时更新的准确数值,只是辅助计算和索引空闲簇
 * 当参考值为0xffffffff 则必须重新计算
 * 占用一整个扇区,但大多数空间不使用
 */
struct FAT32_FSInfo{
    unsigned int  FSI_LeadSig;          //标识符 0x41615252
    unsigned char FSI_Resevered1[480];  //保留
    unsigned int  FSI_StruSig;          //标识符 0x61417272
    unsigned int  FSI_Free_Count;       //上一次记录的空闲簇数量(参考值)
    unsigned int  FSI_Nxt_Free;         //空闲簇的起始搜索位置(参考值)
    unsigned char FSI_Resevered2[12];   //FSInfo占用一个扇区的空间,但大部分都是保留的空间
    unsigned int  FSI_TrailSig;         //结束标志 0xaa550000
}__attribute__((packed));





/* FAT32中的每个FAT表项占用4B 只使用低28位
 *      FAT表项值                  说明
 *      0x0000000           可用簇(还没分配出去)
 * 0x0000002~0xfffffef      已用簇(值为下一个簇的簇号)
 * 0xffffff0~0xffffff6      保留簇(不使用)
 *      0xffffff7           坏簇(不使用)为了避免潜在的风险而不使用
 * 0xffffff8~0xfffffff      文件的最后一个簇
 *
 * FAT表的前2个表项不使用,因此数据区的前两个簇也跟着不使用
 */


/*  根据以上性质,可以对FAT表项性质进行构造  */
enum FATEntryType{
    FAT_free=0,          //未分配的簇
    FAT_using_not_end,   //已分配的簇且不是最后一个簇
    FAT_using_end,       //已分配且是最后一个簇
    FAT_res              //不使用
};


/*  根据簇号判断FAT表项的性质  */
static __attribute__((always_inline))
enum FATEntryType getFATEntryType(unsigned int cluster)
{
    switch (cluster) {
        case 0:
            return FAT_free;
        case 0x0000002 ... 0xfffffef:
            return FAT_using_not_end;
        case 0xffffff0 ... 0xffffff7:
            return FAT_res;
        case 0xffffff8 ... 0xfffffff:
            return FAT_using_end;
    }
    log_to_screen(ERROR,"get Error cluster!");
    return 0;
}




/*  目录项属性  */
#define ATTR_READ_ONLY      (1 << 0)    //只读
#define ATTR_HIDDEN         (1 << 1)    //隐藏
#define ATTR_SYSTEM         (1 << 2)    //系统文件
#define ATTR_VOLUME_ID      (1 << 3)    //卷标
#define ATTR_DIRECTORY      (1 << 4)    //目录
#define ATTR_ARCHIVE        (1 << 5)    //存档
#define ATTR_LONG_NAME (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID) //长文件名


/*  NT字段的值  */
#define LOWERCASE_BASE  (8)
#define LOWERCASE_EXT   (16)


/*  FAT32短目录项(32B)  */
struct FAT32_Directory{
    /*
     * 文件名由8B的基础名和3B的扩展名组成(总共11B)
     * 如果基础名和扩展名的长度不足,则使用空格符号(0x20)来补足
     * 基础名中不允许含有空格,也就是空格只能放到最后
     * 使用ASCII编码,每个字符占用1B
     * 短目录项不区分大小写,都是大写
     * 当Dir_Name[0]=0xE5 0x00 0x05时表明该目录项无效或未使用
     * Dir_Name[0] != 0x20
     */
    unsigned char  Dir_Name[11];     //目录/文件名
    unsigned char  Dir_Attr;         //文件属性
    /*
     * 数值              描述
     * 0x00      基础名大写.扩展名大写
     * 0x08      基础名小写.扩展名大写
     * 0x10      基础名大写.扩展名小写
     * 0x18      基础名小写,扩展名小写
     */
    unsigned char  Dir_NTRes;        //保留
    unsigned char  Dir_CrtTimeTenth; //文件创建时的时间戳
    unsigned short Dir_CrtTime;      //文件创建的时间
    unsigned short Dir_CrtDate;      //文件创建的日期
    unsigned short Dir_LastAccDate;  //最后访问日期
    unsigned short Dir_FatClusHI;    //起始簇号(高位)
    unsigned short Dir_WrtTime;      //最后写入时间
    unsigned short Dir_WrtDate;      //最后写入日期
    unsigned short Dir_FatClusLO;    //起始簇号(低位)
    unsigned int   Dir_FileSize;     //文件大小

    /*
     * 两个簇号合成后 可以确定FAT表项的索引,它同时也是文件或目录在数据区的索引
     */
}__attribute__((packed));



/*  日期结构(2B)  */
struct FAT32_date{
    unsigned short
        day:5,      //取值范围:1~31
        month:4,    //取值范围:1~12
        year:7;     //取值范围:0~127 => (1980 - 2107)
};



static __attribute__((always_inline))
void print_FAT32_date(unsigned short date)
{
    struct FAT32_date date_struct = *(struct FAT32_date *)&date;
    color_printk(GREEN,BLACK ,"%d-%d-%d\n",1980 + date_struct.year,date_struct.month,date_struct.day );
}



/*  时间结构(2B)  */
struct FAT32_time{
    unsigned short
        second:5,   //取值范围:0~29(每2s一次步进)
        minute:6,   //取值范围:0~59
        hour:5;     //取值范围:0~23
};



static __attribute__((always_inline))
void print_FAT32_time(unsigned short time)
{
    struct FAT32_time time_struct = *(struct FAT32_time *)&time;
    color_printk(GREEN,BLACK ,"%d:%d:%d\n",time_struct.hour,time_struct.minute,time_struct.second );
}




/*
 * FAT32长目录项(32B) 长目录项补足了短目录项的缺点
 * 长目录项更多用于记录文件名(5 + 6 + 2)
 * 它不拿来记录日期和时间
 * 使用unicode编码,每个字符占用2B
 * 文件名区分大小写,并且支持多种语言符号
 * 文件名以NULL空字符表示结尾,剩余的空间使用0xffff填充
 */
struct FAT32_LongDirectory{
    unsigned char  LDIR_Ord;        //长目录项的序号
    unsigned short LDIR_Name1[5];   //长文件名的1-5个字符(名称的第一部分)
    unsigned char  LDIR_Attr;       //属性必须为ATTR_LONG_NAME
    unsigned char  LDIR_Type;       //如果为0,说明是长目录项的子项
    unsigned char  LDIR_Chksum;     //短文件名的校验和
    unsigned short LDIR_Name2[6];   //6-11(第二部分)
    unsigned short LDIR_FstClusLO;  //必须为0
    unsigned short LDIR_Name3[2];   //12-13(第三部分)
}__attribute__((packed));







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



/*  FAT32特有的inode信息,用于在VFS抽象层中表明自己是FAT32  */
struct FAT32_inode_info{
    unsigned long first_cluster;    //文件本身的起始簇号,也就是目录项所指向的簇号

    /*
     * 有了这两个参数就可以通过inode快速查找到目录项
     * 从这个目录项里也能查到first_cluster
     */
    unsigned long dentry_location;  //目录项所在的簇(到这个簇里去找)
    unsigned long dentry_position;  //目录项所在簇内的偏移量

    unsigned short create_date;
    unsigned short create_time;
    unsigned short write_date;
    unsigned short write_time;

};



void Disk1_FAT32_FS_init();
unsigned int DISK1_FAT32_read_FAT_Entry(struct FAT32_sb_info* fsbi,unsigned int fat_entry);
unsigned long DISK1_FAT32_write_FAT_Entry(struct FAT32_sb_info* fsbi,unsigned int fat_entry,unsigned int value);
struct super_block* fat32_read_superblock(struct Disk_Partition_Table_Entry* DPTE,void* buf);





#endif