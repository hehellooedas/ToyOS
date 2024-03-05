org 0x10000  ;loader放置在64KB的位置
    jmp Label_Start

%include "./boot/fat12.inc"


;临时的GDT(只需要内核代码段和数据段就好)
[SECTION gdt] 
LABEL_GDT:              dd      0,0     ;NULL描述符
LABEL_DESC_CODE32:      dd      0x0000ffff,0x00cf9a00   ;非一致性,可读,未访问
LABEL_DESC_DATA32:      dd      0x0000ffff,0x00cf9200   ;非一致性,可读写,未访问
;0xcf:1100 1111   粒度为4KB 32位使用EIP 

GdtLen      equ     $ - LABEL_GDT
GdtPtr:     dw      GdtLen - 1
            dd      LABEL_GDT

SelectorCode32      equ     LABEL_DESC_CODE32 - LABEL_GDT   ;代码段选择子
SelectorData32      equ     LABEL_DESC_DATA32 - LABEL_GDT   ;数据段选择子



;临时的64位GDT
[SECTION gdt64]     
LABEL_GDT64:        dq      0x0000000000000000  ;NULL描述符
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

    cli            ;模式切换的时候关闭IF

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
    inc di  ;对比下一个字符
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
    and di,0xffe0
    add di,0x1a
    mov cx,word [es:di]   ;FAT表项的索引
    push cx
    add cx,ax
    add cx,SectorBalance
    mov eax,BaseTmpOfKernelAddr
    mov es,eax
    mov bx,OffsetTmpOfKernelFile  ;kernel临时缓冲区
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
    call Func_ReadOneSector  ;把索引映射的数据区的簇数据取出来
    pop ax    ;kernel.bin在数据区的起始簇号


    push cx
    push eax
    push edi
    push ds
    push esi

    mov cx,0x200        ;512字节
    mov ax,BaseOfKernelFile
    mov fs,ax
    mov edi,dword [OffsetOfKernelFileCount]
    
    mov ax,BaseTmpOfKernelAddr
    mov ds,ax
    mov esi,OffsetTmpOfKernelFile  ;kernel被临时存储的位置


Label_Mov_Kernel:  ;把kernel从临时地址搬移到目标地址(每取出一个簇就搬移一次)
    mov al,byte [ds:esi]
    mov byte [fs:edi],al

    inc esi
    inc edi

    loop Label_Mov_Kernel  ;把内核搬移过去

    mov eax,0x1000
    mov ds,eax

    mov dword [OffsetOfKernelFileCount],edi   ;偏移量edi前面是内核数据

    pop esi
    pop ds
    pop edi
    pop eax
    pop cx

    call Func_GetFATEntry

    cmp ax,0x0fff
    jz Label_File_Loaded
    push ax
    mov dx,RootDirSectors
    add ax,dx
    add ax,SectorBalance
    ;add bx,BPB_BytesPerSec

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
    mov eax,0x0E820          ;子功能号
    mov ecx,20               ;ARDS结构的字节大小
    mov edx,0x534D4150       ;签名标记(固定值SMAP)
    int 0x15
    jc Label_Get_Mem_Fail    ;CF=1表示出错
    add di,20                ;缓冲区往后移动一个ARDS结构的大小
    inc dword [MemStructNumber] ;内存结构体的数量+1

    cmp ebx,0                ;ebx为0意味着这是最后一个ARDS结构(不用动它)
    jne Label_Get_Mem_Struct ;查找下一个ARDS结构
    jmp Label_Get_Mem_OK

Label_Get_Mem_Fail:
    mov dword [MemStructNumber],0
    mov ax,0x1301           
    mov bx,0x008c
    mov dx,0x0500
    mov cx,24

    push ax
    mov ax,ds
    mov es,ax
    pop ax

    mov bp,GetMemStructErrMessage
    int 0x10
    jmp $



Label_Get_Mem_OK:  ;ADRS结构体查找完毕
    mov ax,0x1301           
    mov bx,0x000f
    mov dx,0x0600
    mov cx,29

    push ax
    mov ax,ds
    mov es,ax
    pop ax

    mov bp,GetMemStructOKMessage
    int 0x10



;获取SVGA信息
    mov ax,0x1301
    mov bx,0x000f
    mov dx,0x0800
    mov cx,23

    push ax
    mov ax,ds
    mov es,ax
    pop ax

    mov bp,StartGetSVGAVBEInfoMessage
    int 0x10

    mov ax,0
    mov es,ax
    mov di,0x8000 ;缓冲区(VbeInfoBlock放置的区域)
    mov ax,0x4f00 ;判断是否支持VBE
    int 0x10

    cmp ax,0x004f ;只有返回的ax为0x004f才是支持的
    jz .KO        ;支持VBE功能

;不支持VBE功能
    mov ax,0x1301
    mov bx,0x000f
    mov dx,0x0900
    mov cx,15

    push ax
    mov ax,ds
    mov es,ax
    pop ax

    mov bp,VBE_No_Support
    int 0x10

.KO:
    mov ax,0x1301
    mov bx,0x000f
    mov dx,0x0a00
    mov cx,11

    push ax
    mov ax,ds
    mov es,ax
    pop ax

    mov bp,VBE_Support
    int 0x10

    mov ax,0
    mov es,ax
    mov si,0x800e    ;VideoModePtr指针

    mov esi,[es:si]  ;通过该指针找到VideoModeList(其中每个模式占用2B)
    mov edi,0x8200   ;ModeInfoBlock从该位置开始记录

Label_SVGA_Mode_Info_Get:
    mov cx,word [es:esi]
    
;展示SVGA模式信息
    push ax

    mov ax,0
    mov al,ch
    call Label_DispAL
    
    mov ax,0
    mov al,cl
    call Label_DispAL

    pop ax
 
    cmp cx,0xffff   ;以0XFFFF表示列表的结束
    jz Label_SVGA_Mode_Info_Finish

    mov ax,0x4f01   ;获取指定模式号的VBE显示模式拓展信息
    int 0x10

    cmp ax,0x004f
    jnz Label_SVGA_Mode_Info_Fail  ;如果不是0x004f说明都出错了

    inc dword [SVGAModeCounter]    ;获取到的模式数量+1
    add esi,2      ;每个模式占用2B
    add edi,0x100  ;拓展信息记录在ModeInfoBlock结构里(256B)

    jmp Label_SVGA_Mode_Info_Get



Label_SVGA_Mode_Info_Fail:
    mov ax,0x1301
    mov bx,0x000f
    mov dx,0x0000
    mov cx,19
    
    push ax
    mov ax,ds
    mov es,ax
    pop ax

    mov bp,GetSVGAModeFail
    int 0x10
    jmp $


Label_SET_SVGA_Mode_VESA_VBE_FAIL:
    jmp $


Label_SVGA_Mode_Info_Finish:
    mov ax,0x1301
    mov bx,0x000f
    mov dx,0x0e00
    mov cx,27

    push ax
    mov ax,ds
    mov es,ax
    pop ax

    mov bp,GetSVGAModeInfoMessage
    int 0x10



;设置SVGA模式
    mov ax,0x4f02  ;初始化图形图像控制器并设置VBE显示模式
    mov bx,0x4180
    int 0x10

    cmp ax,0x004f  ;VBE返回状态
    jnz Label_SET_SVGA_Mode_VESA_VBE_FAIL   ;设置失败了




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


;以下是32位的段
[SECTION .s32]
[BITS 32]
Go_To_TMP_Protect:  ;进入临时保护模式
    mov ax,0x10  ;1 0000
    mov ds,ax
    mov es,ax
    mov fs,ax
    mov ss,ax
    mov esp,0x7e00

    call support_long_mode
    test eax,eax  ;如果不支持长模式则eax为0
                  ;test把两个操作数进行and操作,如果eax为0则zf=1
    jz no_support 

    ;处理器支持64位
    ;初始化页表模板
    ;PDPT页目录指针表
    mov dword [0x90000],0x91007
    mov dword [0x90004],0x00000
	mov	dword [0x90800],0x91007
    mov dword [0x90804],0x00000 

    ;PDT页目录项
	mov	dword [0x91000],0x92007
    mov dword [0x91004],0x00000


    ;PT页表项
	mov	dword [0x92000],0x000083
    mov dword [0x92004],0x000000

	mov	dword [0x92008],0x200083
    mov	dword [0x9200c],0x000000


	mov	dword [0x92010],0x400083
    mov dword [0x92014],0x000000

	mov	dword [0x92018],0x600083
    mov dword [0x9201c],0x000000

	mov	dword [0x92020],0x800083
    mov dword [0x92024],0x000000

	mov	dword [0x92028],0xa00083
    mov dword [0x9202c],0x000000


    ;加载64位的GDT
    db 0x66
    lgdt [GdtPtr64]
    mov ax,0x10
    mov ds,ax
    mov es,ax
    mov fs,ax
    mov gs,ax
    mov ss,ax

    mov esp,0x7e00


    ;打开cr4寄存器里的PAE标志位
    mov eax,cr4
    bts eax,5  ;三级分页(PAE)
    mov cr4,eax

    ;加载cr3 设置分页的物理基地址
    mov eax,0x90000
    mov cr3,eax


    ;开启长模式
    mov ecx,0xC0000080  ;ECX中记录读取MSR的地址
    rdmsr       ;EDX:EAX中存储rdmsr返回值

    bts eax,8   ;四级分页
    wrmsr

    ;
    mov eax,cr0
    bts eax,0   ;确保打开保护模式
    bts eax,31  ;开启分页模式
    mov cr0,eax

    ;跳入内核
    jmp SelectorCode64:OffsetOfKernelFile

    



support_long_mode:
    mov eax,0x80000000  ;返回最大的扩展功能号
    cpuid
    cmp eax,0x80000001  ;判断最大功能号的情况
    setnb al   ;al=!CF   只有当CPU的拓展功能号大于等于该值时才能进入长模式
    jb support_long_mode_done   ;如果低于则无法进入

    mov eax,0x80000001  ;返回扩展处理器信息和特性
    cpuid
    bt edx,29  ;edx的第29位为1(支持长模式)则设置CF=1
    setc al    ;al=CF

support_long_mode_done:
    movzx eax,al  ;把al写入到eax然后多余部分变为0
    ret

no_support:
    jmp $




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
    sub al,0x0a  ;字母从A开始的索引
    add al,'A'

.2:
    mov [gs:edi],ax   ;gs已经默认指向了0xb800
    add edi,2
    mov al,dl
    loop .begin

    mov [DisplayPosition],edi  ;定位到下一个可打印字符的位置

    pop edi
    pop edx
    pop ecx

    ret



;------硬盘读取函数------
;input:    @AX:待读取的磁盘起始扇区号
;          @CL:读入的扇区数量
;          @(ES:BX):目标缓冲区起始地址
;no output
Func_ReadOneSector:
push bx
    push ax
    mov dx,0x1f2
    mov al,cl
    out dx,al
    pop ax

    mov dx,0x1f3
    out dx,al

    shr ax,8
    mov dx,0x1f4
    out dx,al

    mov ax,0
    mov dx,0x1f5
    out dx,al

    or al,0xe0
    mov dx,0x1f6
    out dx,al

    mov dx,0x1f7
    mov al,0x20
    out dx,al

.not_ready:
    nop
    in al,dx
    and al,0x88
    cmp al,0x08
    jnz .not_ready


    mov ch,0
    mov ax,cx
    mov dx,256
    mul dx
    mov cx,ax

    mov dx,0x1f0

.go_on_read:
    in ax,dx
    mov [es:bx],ax
    add bx,2
    loop .go_on_read
pop bx
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
    mov bx,BPB_BytesPerSec
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



;数据定义
;临时的IDT
IDT:
    times 0x50  dq  0
IDT_END:

IDT_POINTER:
    dw  IDT_END - IDT - 1
    dd  IDT



;临时变量和字符串定义
SVGAModeCounter:             dd      0
RootDirSizeForLoop:          dw      RootDirSectors
SectorNo:                    dw      0
Odd:                         dw      0 
DisplayPosition:             dd      0
OffsetOfKernelFileCount:     dd      OffsetOfKernelFile
StartLoaderMessage:          db      "Start Loader"
KernelFileName:              db      'KERNEL  BIN',0
NoKernelMessage:             db      'ERROR:NO Kernel FOUND!'
StartGetMemStructMessage:    db      'Start Get Memory Struct.'
GetMemStructErrMessage:      db      'Get Memory Struct ERROR!'
GetMemStructOKMessage:       db      'Get Memory Struct SUCCESSFUL!'
MemStructNumber:             dd      0
VBE_No_Support:              db      'No VBE SUPPORT!'
VBE_Support:                 db      'VBE SUPPORT'
StartGetSVGAVBEInfoMessage:  db      'Start Get SVGA VBE Info'
GetSVGAModeFail:             db      'Get SVGA Mode Fail!'
GetSVGAModeInfoMessage:      db      'Get SVGA Mode Successfully!'

;times (0x1000 - ($ - $$)) db 0
