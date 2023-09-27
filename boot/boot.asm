org 0x7c00  ;OBR被放置在0x7c00


RootDirSectors          equ     14   ;根目录占用的扇区数
SectorNumOfRootDirStart equ     19   ;根目录的起始扇区号
SectorNumOfFat1Start    equ     1    ;FAT1表的起始扇区号(0号扇区是MBR)
SectorBalance           equ     17   ;平衡文件起始簇号和数据区起始簇号的差值(19-2)

BaseOfStack       equ      0x7c00    ;boot的栈空间
BaseOfLoader      equ      0x1000    ;loader放置的位置(段地址)
OffsetOfLoader    equ      0x00      ;偏移地址


;FAT12文件系统的组成结构信息
    jmp short Label_Start        ;跳转指令(BS_jmpBoot)
    nop  ;BS_jmpBoot需要占用3个字节,nop充当一个填充符

    BS_OEMNAME          db       'April   '    ;OEM厂商名
    BPB_BytesPerSec     dw       512           ;每个扇区的字节数
    BPB_SecPerClus      db       1             ;每个簇的扇区数
    BPB_RsvdSecCnt      dw       1             ;保留扇区数(第一个扇区)
    BPB_NumFATs         db       2             ;FAT表的份数
    BPB_RootEntCnt      dw       224           ;根目录可容纳的目录项数(固定)
    BPB_TotSec16        dw       2880          ;总扇区数(刚好是3.5英寸软盘的容量)
    BPB_Media           db       0xf0          ;介质描述符
    BPB_FATSz16         dw       9             ;每个FAT扇区数
    BPB_SecPerTrk       dw       18            ;每磁道扇区数
    BPB_NumHeads        dw       2             ;磁头数
    BPB_hiddSec         dd       0             ;隐藏扇区数
    BPB_TotSec32        dd       0             ;总扇区数(BPB_TotSec16=0时使用)
    BPB_DrvNum          db       0             ;0x13号中断的驱动器号
    BPB_Reserved1       db       0             ;预留字段
    BPB_BootSig         db       0x29          ;拓展引导标记(固定值)
    BPB_VolID           dd       0             ;卷序列号
    BS_VolLab           db       'boot loader' ;卷标
    BS_FileSysType      db       'FAT12   '    ;文件系统类型


[BITS 16]
Label_Start:
    mov ax,cs
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov sp,BaseOfStack

;------清屏------
    mov ax,0x0600
    mov bx,0x0700
    mov cx,0
    mov dx,0x0184
    int 0x10

;------设置光标位置------
    mov ax,0x0200
    mov bx,0
    mov dx,0
    int 0x10

;------往屏幕输出信息------
    mov ax,0x1301           
    mov bx,0x000f           ;白色高亮
    mov dx,0x0000           ;打印在第一行
    mov cx,10               ;字符串的长度
    mov es,dx
    mov bp,StartBootMessage ;打印字符的地址
    int 0x10


;------复位软盘驱动器 ------
    xor ah,ah  ;功能号
    xor dl,dl  ;第一个软盘
    int 0x13


;------寻找loader.bin------
    mov word [SectorNo],SectorNumOfRootDirStart  ;从根目录起始扇区开始检查
Label_Search_In_Root_Dir_Begin:
    cmp word [RootDirSizeForLoop],0  ;根目录占用的扇区数(14)
    jz Label_No_LoaderBin  ;如果根目录没有占用扇区,说明loader不可能存在
    dec word [RootDirSizeForLoop]    ;每找一次就-1
    mov ax,0
    mov es,ax
    mov bx,0x8000      ;读出来的扇区放置在内存中的地址
    mov ax,[SectorNo]  ;当前检查的是第几个扇区
    mov cl,1           ;每次只读一个扇区
    call Func_ReadOneSector
    mov si,LoaderFileName  ;loader的文件名
    mov di,0x8000
    cld
    mov dx,0x10   ;每个根目录扇区可容纳的目录项个数(去目录项里寻找)

Label_Search_For_LoaderBin:
    cmp dx,0      ;如果当前扇区的目录项都比对完了,就去下一个扇区里找
    jz Label_Goto_Next_Sector_In_Boot_Dir
    dec dx        ;每对比失败一次就-1
    mov cx,11     ;文件名+文件拓展名共11字节

Label_Cmp_FileName:
    cmp cx,0      ;11个字符都对比成功,找到了
    jz Label_FileName_Found
    dec cx        ;进行下一个字符的对比
    lodsb         ;[ds:si] -> al
    cmp al,byte [es:di]  ;字符相同则对比下一个字符
    jz Label_Go_On
    jmp Label_Different  ;当前字符对比失败就进行下一次比对

Label_Go_On:
    inc di
    jmp Label_Cmp_FileName

Label_Different:
    and di,0xffe0  ;1111 1111 1110 0000
    add di,0x20    ;0000 0000 0010 0000 (去该扇区里的下一个32字节找)
    mov si,LoaderFileName  ;si一直指向文件名
    jmp Label_Search_For_LoaderBin

Label_Goto_Next_Sector_In_Boot_Dir:
    add word [SectorNo],1   ;去下一个扇区寻找
    jmp Label_Search_In_Root_Dir_Begin

;查找失败提示错误
Label_No_LoaderBin:
    mov ax,0
    mov es,ax
    mov ax,0x1301
    mov bx,0x008c
    mov dx,0x0100
    mov cx,22
    
    mov bp,NoLoaderMessage
    int 0x10
    jmp $




Label_FileName_Found:
    mov ax,RootDirSectors
    and di,0xffe0
    add di,0x1a   ;起始簇号(26-27位)查找文件数据的起点
    mov cx ,word [es:di]
    push cx  ;FAT表项的索引(由于两个函数都要用到AX,所以一些信息就存储在栈里)
    add cx,ax     ;越过根目录区 数据区起始扇区=根目录起始扇区+根目录所占用扇区-2
    add cx,SectorBalance   ;确定loader.bin在磁盘中的位置
    mov ax,BaseOfLoader
    mov es,ax              ;把loader.bin读取到物理地址的段地址
    mov bx,OffsetOfLoader  ;把loader.bin读取到物理地址的偏移地址
    mov ax,cx

Label_Go_On_Loading_File:
    push ax
    push bx
    mov ah,0x0e
    mov al,'.'
    mov bl,0x0f
    int 0x10
    pop bx
    pop ax

    mov cl,1
    call Func_ReadOneSector  ;第一个簇必然是,所以把它映射的数据区的一个簇的数据给拿出来
    pop ax    ;loader.bin在数据区的起始簇号
    call Func_GetFATEntry
    cmp ax,0x0fff   ;该标志表示文件的最后一个簇
    jz Label_File_Loaded  ;如果已经找完则跳转
    push ax
    mov dx,RootDirSectors
    add ax,dx
    add ax,SectorBalance
    add bx,[BPB_BytesPerSec]      ;如果不止一个簇,则搬移到下一个簇大小的地址
    jmp Label_Go_On_Loading_File  ;进入下一个簇

Label_File_Loaded:
    jmp BaseOfLoader:OffsetOfLoader  ;跳转到loader.bin



;临时变量
RootDirSizeForLoop:dw RootDirSectors
SectorNo:          dw 0
Odd:               dw 0

;显示信息
StartBootMessage:  db 'Start Boot'
NoLoaderMessage:   db 'ERROR:NO LOADER FOUND!'
LoaderFileName:    db 'LOADER  BIN',0
;文件名8字节(不足8字节的拿空格填充),拓展名3字节




;--------软盘读取函数(0x13中断)--------
;input:    @AX:待读取的磁盘起始扇区号
;          @CL:读入的扇区数量
;          @(ES:BX):目标缓冲区起始地址
;no output
Func_ReadOneSector:
    push bp
    mov bp,sp
    sub esp,2
    mov byte [bp - 2],cl
    push bx
    mov bl,[BPB_SecPerTrk]
    div bl     ;LBA扇区号 / 每磁道扇区数
    inc ah     ;扇区号
    mov cl,ah
    mov dh,al
    shr al,1   ;柱面号
    mov ch,al
    and dh,1   ;磁头号(0/1)
    pop bx
    mov dl,[BPB_DrvNum]  ;驱动器号

Label_Go_ON_Reading:
    mov ah,2
    mov al, byte [bp - 2] ;读入扇区数从cl转到了al
    int 0x13
    jc Label_Go_ON_Reading
    add esp,2
    pop bp
    ret




;--根据当前FAT表项索引得出下一个FAT表项--
;input:   @AX=当前FAT表项索引
;output:  @AX=下一个FAT表项索引
Func_GetFATEntry:
    push es
    push bx
    push ax
    mov ax,0
    mov es,ax
    pop ax
    mov byte [Odd],0
    mov bx,3
    mul bx
    mov bx,2
    div bx
    cmp dx,0
    jz Lable_Even
    mov byte [Odd],1

Lable_Even:
    xor dx,dx
    mov bx,[BPB_BytesPerSec]
    div bx
    push dx   ;FAT表项在该扇区中的偏移地址
    mov bx,0x8000
    add ax,SectorNumOfFat1Start  ;FAT表项在哪一个扇区
    mov cl,2  ;读入两个扇区
    call Func_ReadOneSector

    pop dx
    add bx,dx      ;FAT表项在内存中的位置
    mov ax,[es:bx] ;一次性拿出两个字节
    cmp byte [Odd],1
    jnz Label_Even_2
    shr ax,4  ;如果是奇数项则需要位移

Label_Even_2:
    and ax,0x0fff   ;0000 1111 1111 1111(1.5B~)
    pop bx
    pop es
    ret    

times 510 - ($ - $$) db 0
db 0x55,0xaa  ;结束标志