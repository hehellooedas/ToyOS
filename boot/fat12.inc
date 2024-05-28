;Data for Bootloader about FAT12 filesystem
;FAT系列文件系统简单、稳定、兼容性强，适合作为启动盘的文件系统
;|------------------------|
;|     MBR(1 sector)      |
;|------------------------|
;|   FAT Table 1(2-10)    |     ;FAT表里每一个项目叫FAT表项(4B)
;|------------------------|
;|   FAT Table 2(11-18)   |     ;表项的值指向下一个表项或代表结束/表项本身是索引
;|------------------------|
;|    RootDir(19-33)      |     ;根目录区每一个项目叫目录项(FAT n代表了表项的大小为n位)
;|------------------------|
;|   DataSection(34-~)    |
;|________________________|
;
;FAT Table 2 用于备份,可以在关机时把FAT1赋值给FAT2



;根目录区(总共14扇区)用于存储目录项(每个32B)
;|-----------------------------|
;|    文件名(8) + 拓展名(3)      |
;|-----------------------------|
;|         文件属性(1)          |
;|-----------------------------|
;|          保留(10)           |
;|-----------------------------|
;|      最后一次写入时间(2)       |
;|-----------------------------|
;|      最后一次写入日期(2)       |
;|-----------------------------|
;|        起始簇号(2)           |  ->  FAT表项的索引
;|-----------------------------|
;|        文件大小(4)           |
;|-----------------------------|



;Fat12表项位宽是12位,一个表项对应了一个簇
;FAT[0],FAT[1]保留与占用(不重要、不使用)
;FAT[n] = 0x000 -> 该簇未使用
;         0x002 ~ 0xfef -> 标识下一个簇的簇号
;         0xff0 ~ 0xff6 -> 保留簇
;         0xff7 -> 坏簇(簇中的数据不可靠)
;         0xff8 ~ 0xfff -> 文件的最后一个簇(为了兼容性选择用完12位)


BPB_BytesPerSec     equ       512           ;每个扇区的字节数
BPB_SecPerClus      equ       1             ;每个簇的扇区数
BPB_RsvdSecCnt      equ       1             ;保留扇区数(第一个扇区)
BPB_NumFATs         equ       2             ;FAT表的份数
BPB_RootEntCnt      equ       224           ;根目录可容纳的目录项数(固定)
BPB_TotSec16        equ       4096          ;总扇区数
BPB_Media           equ       0xf8          ;介质描述符
BPB_FATSz16         equ       9             ;每个FAT扇区数
BPB_SecPerTrk       equ       18            ;每磁道扇区数
BPB_NumHeads        equ       2             ;磁头数
BPB_hiddSec         equ       0             ;隐藏扇区数
BPB_TotSec32        equ       0             ;总扇区数(BPB_TotSec16=0时使用)
BS_DrvNum          equ        0             ;0x13号中断的驱动器号
BS_Reserved1       equ        0             ;预留字段
BS_BootSig         equ        0x29          ;拓展引导标记(固定值)
BS_VolID           equ        0             ;卷序列号

RootDirSectors          equ     14   ;根目录占用的扇区数
SectorNumOfRootDirStart equ     19   ;根目录的起始扇区号
SectorNumOfFat1Start    equ     1    ;FAT1表的起始扇区号(0号扇区是MBR)
SectorBalance           equ     17   ;平衡文件起始簇号和数据区起始簇号的差值(19-2)


;boot.asm
BaseOfStack       equ      0x7c00    ;boot的栈空间
BaseOfLoader      equ      0x1000    ;loader放置的位置(段地址)
OffsetOfLoader    equ      0x00      ;偏移地址



;loader.asm
BaseOfKernelFile        equ      0x00
OffsetOfKernelFile      equ      0x100000 ;1MB
BaseTmpOfKernelFile     equ      0x00
OffsetTmpOfKernelFile   equ      0x0000   ;临时地址
BaseTmpOfKernelAddr     equ      0x9000
MemoryStructBufferAddr  equ      0x7e00   ;ARDS缓冲区地址