#ifndef __KERNEL_PTRACE_H
#define __KERNEL_PTRACE_H



#define PT_SIZE     sizeof(struct pt_regs)
#define PT_R15      0x00
#define PT_R14      0x08
#define PT_R13      0x10
#define PT_R12      0x18
#define PT_R11      0x20
#define PT_R10      0x28
#define PT_R9       0x30
#define PT_R8       0x38
#define PT_RBX      0x40
#define PT_RCX      0x48
#define PT_RDX      0x50
#define PT_RSI      0x58
#define PT_RDI      0x60
#define PT_RBP      0x68
#define PT_RS       0x70
#define PT_ES       0x78
#define PT_RAX      0x80
#define PT_FUNC     0x88
#define PT_ERRCODE  0x90
#define PT_RIP      0x98
#define PT_CS       0xa0
#define PT_RFLAGS   0xa8
#define PT_RSP      0xb0
#define PT_SS       0xb8



struct pt_regs{
    unsigned long r15;
    unsigned long r14;
    unsigned long r13;
    unsigned long r12;
    unsigned long r11;
    unsigned long r10;
    unsigned long r9;
    unsigned long r8;
    unsigned long rbx;
    unsigned long rcx;
    unsigned long rdx;
    unsigned long rsi;
    unsigned long rdi;
    unsigned long rbp;
    unsigned long ds;
    unsigned long es;
    unsigned long rax;
    
    /*  下面几个就不用pop了(直接addq $0x38,%rsp绕过去)  */
    unsigned long func;
    unsigned long errcode;
    unsigned long rip;
    unsigned long cs;
    unsigned long rflags;
    unsigned long rsp;
    unsigned long ss;
};



#endif // !1