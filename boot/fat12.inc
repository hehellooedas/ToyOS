

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