BPB_BytesPerSec     equ       512           ;每个扇区的字节数
BPB_SecPerClus      equ       1             ;每个簇的扇区数
BPB_RsvdSecCnt      equ       1             ;保留扇区数(第一个扇区)
BPB_NumFATs         equ       2             ;FAT表的份数
BPB_RootEntCnt      equ       224           ;根目录可容纳的目录项数(固定)
BPB_TotSec16        equ       2880          ;总扇区数(刚好是3.5英寸软盘的容量)
BPB_Media           equ       0xf0          ;介质描述符
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
OffsetTmpOfKernelFile   equ      0x7e00   ;临时地址
BaseTmpOfKernelAddr     equ      0x00
MemoryStructBufferAddr  equ      0x7e00   ;ARDS缓冲区地址