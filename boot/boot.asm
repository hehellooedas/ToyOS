org 0x7c00  ;OBR被放置在0x7c00


;数据定义不占磁盘空间
BaseOfStack       equ      0x7c00    ;boot的栈空间
BaseOfLoader      equ      0x1000    ;loader放置的位置
OffsetOfLoader    equ      0x00      ;


RootDirSectors          equ     14   ;根目录占用的扇区数
SectorNumOfRootDirStart equ     19   ;根目录的起始扇区号
SectorNumOfFat1Start    equ     1    ;FAT1表的起始扇区号(0号扇区是MBR)
SectorBalance           equ     17   ;平衡文件起始簇号和数据区起始簇号的差值(19-2)


    jmp short Label_Start        ;跳转指令
    nop  ;BS_jmpBoot需要占用3个字节,nop充当一个填充符

    ;FAT12文件系统的组成结构信息
    BS_OEMNAME          db       'April   '    ;OEM厂商名
    BPB_BytesPerSec     dw       512           ;每个扇区的字节数
    BPB_SecPerClus      db       1             ;每个簇的扇区数
    BPB_RsvdSecCnt      dw       1             ;保留扇区数(第一个扇区)
    BPB_NameFATs        db       2             ;FAT表的份数
    BPB_RootEntCnt      dw       224           ;根目录可容纳的目录项数
    BPB_TotSec16        dw       2880          ;总扇区数(刚好是3.5英寸软盘的容量)
    BPB_Media           db       0xf0          ;介质描述符
    BPB_FATSz16         dw       9             ;每个FAT扇区数
    BPB_SecPerTrk       dw       18            ;每磁道扇区数
    BPB_NumHeads        dw       2             ;磁头数
    BPB_hiddSec         dd       0             ;隐藏扇区数
    BPB_TotSec32        dd       0             ;总扇区数(BPB_TotSec16无效时使用)
    BPB_DrvNum          db       0             ;驱动器号
    BPB_Reserved1       db       0             ;预留字段
    BPB_BootSig         db       0x29          ;拓展引导标记(固定值)
    BPB_VolID           dd       0             ;卷序列号
    BS_VolLab           db       'boot loader' ;卷标
    BS_FileSysType      db       'FAT12   '    ;文件系统类型



Label_Start:
    mov ax,cs
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov sp,BaseOfStack

;清屏
    mov ax,0x0600
    mov bx,0x0700
    mov cx,0
    mov dx,0x0184
    int 0x10

;设置光标位置
    mov ax,0x0200
    mov bx,0
    mov dx,0
    int 0x10

;往屏幕输出信息
    mov ax,0x1301
    mov bx,0x000f
    mov dx,0
    mov cx,10
    mov es,dx
    mov bp,StartBootMessage
    int 0x10

;复位软盘驱动器 
    xor ah,ah  ;功能号
    xor dl,dl  ;第一个软盘
    int 0x13


;寻找loader.bin
    mov word [SectorNo],SectorNumOfRootDirStart
Label_Search_In_Root_Dir_Begin:
    cmp word [RootDirSizeForLoop],0  ;根目录占用的扇区数(14)
    jz Label_No_LoaderBin  ;如果找不到
    dec word [RootDirSizeForLoop]    ;每找一次就-1
    mov ax,0
    mov es,ax
    mov bx,0x8000
    mov ax,[SectorNo]
    mov cl,1
    call Func_ReadOneSector
    mov si,LoaderFileName
    mov di,0x8000
    cld
    mov dx,0x10   ;每个扇区可容纳的目录项个数

Label_Search_For_LoaderBin:
    cmp dx,0      ;如果当前扇区的目录项都比对完了,就去下一个扇区里找
    jz Label_Goto_Next_Sector_In_Boot_Dir
    dec dx        ;每对比失败一次就-1
    mov cx,11     ;目录项的文件名长度

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
    and di,0xffe0
    add di,0x20
    mov si,LoaderFileName
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
    add di,0x1a
    mov cx ,word [es:di]
    push cx
    add cx,ax
    add cx,SectorBalance
    mov ax,BaseOfLoader
    mov es,ax
    mov bx,OffsetOfLoader
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
    call Func_ReadOneSector
    pop ax
    call Func_GetFATEntry
    cmp ax,0x0fff
    jz Label_File_Loaded
    push ax
    mov dx,RootDirSectors
    add ax,dx
    add ax,SectorBalance
    add bx,[BPB_BytesPerSec]
    jmp Label_Go_On_Loading_File

Label_File_Loaded:
    jmp BaseOfLoader:OffsetOfLoader  ;跳转到loader.bin



;临时变量
RootDirSizeForLoop:dw RootDirSectors
SectorNo:          dw 0
Odd:               dw 0

;显示信息
StartBootMessage:  db 'Start Boot'
NoLoaderMessage:   db 'ERROR:NO LOADER FOUND!'
LoaderFileName:    db 'LOADER BIN',0


times 510 - ($ - $$) db 0
db 0x55,0xaa



;软盘读取函数(0x13中断)
;@AX:待读取的磁盘起始扇区号
;@CL:读入的扇区数量
;@(ES:BX):目标缓冲区起始地址
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



;解析FAT表
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
    push dx
    mov bx,0x8000
    add ax,SectorNumOfFat1Start
    mov cl,2
    call Func_ReadOneSector

    pop dx
    add bx,dx
    mov ax,[es:bx]
    cmp byte [Odd],1
    jnz Label_Even_2
    shr ax,4

Label_Even_2:
    and ax,0x0fff
    pop bx
    pop es
    ret    

