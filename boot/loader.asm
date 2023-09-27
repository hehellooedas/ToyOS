org 0x10000  ;loader放置在64KB的位置
    jmp Label_Start

%include "./boot/fat12.inc"


[SECTION gdt] ;临时的GDT
LABEL_GDT:              dd      0,0
LABEL_DESC_CODE32:      dd      0x0000ffff,0x00cf9a00
LABEL_DESC_DATA32:      dd      0x0000ffff,0x00cf9200
;0xcf:1100 1111   粒度为4KB 32位使用EIP 


GdtLen      equ     $ - LABEL_GDT
GdtPtr:     dw      GdtLen - 1
            dd      LABEL_GDT

SelectorCode32      equ     LABEL_DESC_CODE32 - LABEL_GDT   ;代码段选择子
SelectorData32      equ     LABEL_DESC_DATA32 - LABEL_GDT   ;数据段选择子


[SECTION gdt64]
LABEL_GDT64:        dq      0x0000000000000000
LABEL_DESC_CODE64:  dq      0x0020980000000000
LABEL_DESC_DATA64:  dq      0x0000920000000000

GdtLen64      equ     $ - LABEL_GDT64
GdtPtr64:     dw      GdtLen64 - 1
              dd      LABEL_GDT64
SelectorCode64      equ    LABEL_DESC_CODE64 - LABEL_GDT64
SelectorData64      equ    LABEL_DESC_DATA64 - LABEL_GDT64




[SECTION .s16]
[BITS 16]
Label_Start:
;------初始化寄存器------
    mov ax,cs
    mov ds,ax
    mov es,ax
    mov ax,0x00
    mov ss,ax
    mov sp,0x7c00

;------打印loader引导标记------
    mov ax,0x1301
    mov bx,0x000f
    mov dx,0x0200
    mov cx,12

    push ax
    mov ax,ds
    mov es,ax
    pop ax

    mov bp,StartLoaderMessage
    int 0x10
    


;------打开A20地址线------
    push ax
    in al,0x92
    or al,0000_0010b
    out 0x92,al     ;打开后可以访问1MB以上的内存地址
    pop ax

    cli

    db 0x66        ;以32位方式寻址
    lgdt [GdtPtr]

    mov eax,cr0
    or eax,1
    mov cr0,eax   ;开启保护模式

    mov ax,SelectorData32
    mov fs,ax     ;让fs在实模式下的寻址能力超过1MB


    mov eax,cr0
    and al,1111_1110b
    mov cr0,eax       ;关闭保护模式

    sti

;------复位软盘驱动器 ------
    xor ah,ah
    xor dl,dl
    int 0x13


;--------搜索kernel.bin---------
    mov word [SectorNo],SectorNumOfRootDirStart  ;从根目录起始扇区开始检查
Label_Search_In_Root_Dir_Begin:
    cmp word [RootDirSizeForLoop],0  ;根目录占用的扇区数(14)
    jz Label_No_KernelBin  ;根目录里所有扇区都找完了没找到说明失败了
    dec word [RootDirSizeForLoop]    ;每找一次就-1
    mov ax,0
    mov es,ax
    mov bx,0x8000      ;读出来的扇区放置在内存中的地址
    mov ax,[SectorNo]  ;当前检查的是第几个扇区
    mov cl,1           ;每次只读一个扇区
    call Func_ReadOneSector
    mov si,KernelFileName
    mov di,0x8000
    cld
    mov dx,0x10   ;每个根目录扇区可容纳的目录项个数(去目录项里寻找)

Label_Search_For_KernelBin:
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
    mov si,KernelFileName  ;si一直指向文件名
    jmp Label_Search_For_KernelBin

Label_Goto_Next_Sector_In_Boot_Dir:
    add word [SectorNo],1   ;去下一个扇区寻找
    jmp Label_Search_In_Root_Dir_Begin

;查找失败提示错误
Label_No_KernelBin:
    mov ax,0x1301
    mov bx,0x008c
    mov dx,0x0300
    mov cx,22

    push ax
    mov ax,ds
    mov es,ax
    pop ax
    
    mov bp,NoKernelMessage
    int 0x10
    jmp $

;找到kernel文件名后
Label_FileName_Found:
    mov ax,RootDirSectors
    mov di,0xffe0
    add di,0x1a
    mov cx,word [es:di]   ;FAT表项的索引
    push cx
    add cx,ax
    add cx,SectorBalance
    mov eax,BaseTmpOfKernelAddr
    mov es,eax
    mov bx,OffsetTmpOfKernelFile
    mov ax,cx

Label_Go_On_Kernel_File:
    push ax
    push bx
    mov ah,0x0e
    mov al,'.'
    mov bl,0x0f
    int 0x10
    pop bx
    pop ax

    mov cl,1
    call Func_ReadOneSector  ;把第一个索引映射的数据区的簇数据取出来
    pop ax    ;kernel.bin在数据区的起始簇号


    push cx
    push eax
    push fs
    push edi
    push ds
    push esi

    mov cx,0x200
    mov ax,BaseOfKernelFile
    mov fs,ax
    mov edi,dword [OffsetOfKernelFileCount]
    
    mov ax,BaseTmpOfKernelAddr
    mov ds,ax
    mov esi,OffsetTmpOfKernelFile  ;kernel被临时存储的位置

Label_Mov_Kernel:  ;把kernel从临时地址搬移到目标地址
    mov al,byte [ds:esi]
    mov byte [fs:edi],al

    inc esi
    inc edi

    loop Label_Mov_Kernel  ;把内核搬移过去

    mov eax,0x1000
    mov ds,eax

    mov dword [OffsetOfKernelFileCount],edi   ;偏移量edi之前已经是内核数据了

    pop esi
    pop ds
    pop edi
    pop fs
    pop eax
    pop cx

    call Func_GetFATEntry

    cmp ax,0x0fff
    jz Label_File_Loaded
    push ax
    mov dx,RootDirSectors
    add ax,dx
    add ax,SectorBalance

    jmp Label_Go_On_Kernel_File


Label_File_Loaded:
    mov ax,0xb800
    mov gs,ax
    mov ah,0xf
    mov al,'G'
    mov [gs:(80 * 0 + 39 * 2)],ax

;------关闭软驱马达------  ;软盘落幕
KillMotor:
    push dx
    mov dx,0x3f2
    mov al,0
    out dx,al
    pop dx


;------获取内存地址尺寸类型------
    mov ax,0x1301
    mov bx,0x000f
    mov dx,0x0400
    mov cx,24
    push ax
    mov ax,ds
    mov es,ax
    pop ax
    mov bp,StartGetMemStructMessage
    int 0x10

;为获取内存布局作准备
    mov ebx,0   ;ARDS后续值,初始化为0
    mov ax,0
    mov es,ax  ;缓冲区段地址
    mov di,MemoryStructBufferAddr   ;缓冲区偏移地址

Label_Get_Mem_Struct:
    mov eax,0xE820           ;子功能号
    mov ecx,20               ;ARDS结构的字节大小
    mov edx,0x534D4150       ;签名标记
    int 0x15
    jc Label_Get_Mem_Fail    ;CF=1表示出错
    add di,20                ;缓冲区往后移动一个ARDS结构的大小
    cmp ebx,0                ;ebx为0意味着这是最后一个ARDS结构(不用动它)
    jne Label_Get_Mem_Struct ;查找下一个ARDS结构
    jmp Label_Get_Mem_OK

Label_Get_Mem_Fail:
    mov ax,0x1301           
    mov bx,0x008c
    mov dx,0x0500
    mov cx,24
    push ax
    mov ax,ds
    mov es,ax
    push ax
    mov bp,GetMemStructErrMessage
    int 0x10
    jmp $

Label_Get_Mem_OK:
    mov ax,0x1301           
    mov bx,0x000f
    mov dx,0x0600
    mov cx,29
    push ax
    mov ax,ds
    mov es,ax
    pop ax
    mov bp,Label_Get_Mem_OK
    int 0x10

;设置SVGA模式
    jmp $
    ;mov ax,0x4f02
    ;mov bx,0x4180
    ;int 0x10

    ;cmp ax,0x004f
    ;jnz Label_SET_SVGA_Mode_VESA_VBE_FAIL




;初始化gdt和idt后进入保护模式
    cli

    db 0x66
    lgdt [GdtPtr]

    db 0x66
    lidt [IDT_POINTER]

    mov eax,cr0
    or eax,1
    mov cr0,eax

    jmp dword SelectorCode32:Go_To_TMP_Protect  ;跳入保护模式



;数据定义
;临时的IDT
IDT:
    times 0x50  dq  0
IDT_END:

IDT_POINTER:
    dw  IDT_END - IDT - 1
    dd  IDT

RootDirSizeForLoop:          dw      RootDirSectors
SectorNo:                    dw      0
Odd:                         dw      0 
DisplayPosition:             dd      0
OffsetOfKernelFileCount:     dw      OffsetTmpOfKernelFile
StartLoaderMessage:          db      "Start Loader"
KernelFileName:              db      'KERNEL  BIN',0
NoKernelMessage:             db      'ERROR:NO Kernel FOUND!'
StartGetMemStructMessage:    db      'Start Get Memory Struct.'
GetMemStructErrMessage:      db      'Get Memory Struct ERROR!'
GetMemStructOKMessage:       db      'Get Memory Struct SUCCESSFUL!'

BPB_SecPerTrk       dw       18            ;每磁道扇区数
BPB_DrvNum          db       0             ;0x13号中断的驱动器号
BPB_BytesPerSec     dw       512           ;每个扇区的字节数

[SECTION .s16lib]
[BITS 16]
;----- 显示16进制的数字------
;input:    @AL=要显示的16进制数字
;output
Label_DispAL:
    push ecx
    push edx
    push edi

    mov edi,[DisplayPosition]  ;打印位置的偏移地址
    mov ah,0x0f
    mov dl,al
    shr al,4
    mov ecx,2

.begin:
    and al,0x0f  ;0000 1111
    cmp al,9     ;0000 1001 判断待打印的16进制数和9的关系
    ja .1        ;如果大于9则要打印的是字母,要进行转换
    add al,'0'   ;如果小于等于9只需要加上数字0的ASCII就好了
    jmp .2    

.1:
    sub al,0x0a
    add al,'A'

.2:
    mov [gs:edi],ax   ;gs已经默认指向了0xb800
    add edi,2
    mov al,dl
    loop .begin

    mov [DisplayPosition],edi

    pop edi
    pop edx
    pop ecx

    ret



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
    push dx   ;FAT表项的偏移扇区号
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
    and ax,0x0fff   ;0000 1111 1111 1111
    pop bx
    pop es
    ret    




[SECTION .s32]
[BITS 32]
Go_To_TMP_Protect:  ;进入临时长模式
    mov ax,0x10  ;1 0000
    mov ds,ax
    mov es,ax
    mov fs,ax
    mov ss,ax
    mov esp,0x7e00

    call support_long_mode
    test eax,eax

    jz no_support

support_long_mode:
    mov eax,0x80000000
    cpuid
    cmp eax,0x80000001
    setnb al
    jb support_long_mode_done
    mov eax,0x80000001
    cpuid
    bt edx,29  ;edx的第29位为1则设置CF=1
    setc al    ;al=CF

support_long_mode_done:
    movzx eax,al  ;把al写入到eax然后多余部分变为0
    ret

no_support:
    jmp $