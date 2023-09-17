org 0x10000

    mov ax,cs
    mov ds,ax
    mov es,ax
    mov ax,0x00
    mov ss,ax
    mov sp,0x7c00


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
    jmp $

StartLoaderMessage:
    db  "Start Loader"