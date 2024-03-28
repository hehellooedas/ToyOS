#ifndef __KERNEL_GATE_H
#define __KERNEL_GATE_H


struct desc_struct{
    unsigned char x[8];
};


struct gate_struct{
    unsigned char x[16];
};


extern struct desc_struct GDT_Table[];
extern struct gate_struct IDT_Table[];
extern unsigned int TSS64_Table[26];



/*  设置门描述符,attr决定了这是什么门  */
#define _set_gate(gate_selector_addr,attr,ist,code_addr)            \
do{ unsigned long __d0,__d1;                                        \
    asm volatile("movw %%dx,%%ax     \n\t"                          \
                 "andq $0x7,%%rcx    \n\t"                          \
                 "addq %4,%%rcx      \n\t"                          \
                 "shlq $32,%%rcx     \n\t"                          \
                 "addq %%rcx,%%rax   \n\t"                          \
                 "xorq %%rcx,%%rcx   \n\t"                          \
                 "movl %%edx,%%ecx   \n\t"                         \
                 "shrq $16,%%rcx     \n\t"                          \
                 "shlq $48,%%rcx     \n\t"                          \
                 "addq %%rcx,%%rax   \n\t"                          \
                 "movq %%rax,%0      \n\t"                          \
                 "shrq $32,%%rdx     \n\t"                          \
                 "movq %%rdx,%1      \n\t"                          \
                :"=m"(*((unsigned long*)(gate_selector_addr))),      \
                 "=m"(*(1 + (unsigned long*)(gate_selector_addr))),  \
                 "=&a"(__d0),"=&d"(__d1)                            \
                :"i"(attr << 8),                                    \
                 "3"((unsigned long *)(code_addr)),                 \
                 "2"(0x8 << 16),                                    \
                 "c"(ist)                                           \
                :"memory"                                           \
    ); \
}while(0)



/*  设置TR  */
#define load_TR(n)          \
do{                         \
    asm volatile (          \
        "ltr %%ax"          \
        :                   \
        :"a"(n << 3)        \
        :"memory"           \
    );                      \
}while(0)



/*  
 * 设置指定Table的TSS
 * 其中reserved部分不需要赋值
*/
static __attribute__((always_inline)) 
void set_tss64(unsigned int* Table,unsigned long rsp0,unsigned long rsp1,unsigned long rsp2\
,unsigned long ist1,unsigned long ist2,unsigned long ist3,unsigned long ist4\
,unsigned long ist5,unsigned long ist6,unsigned long ist7)
{
    *(unsigned long*)(Table + 1) = rsp0;
    *(unsigned long*)(Table + 3) = rsp1;
    *(unsigned long*)(Table + 5) = rsp2;

    *(unsigned long*)(Table + 9) = ist1;
    *(unsigned long*)(Table + 11) = ist2;
    *(unsigned long*)(Table + 13) = ist3;
    *(unsigned long*)(Table + 15) = ist4;
    *(unsigned long*)(Table + 17) = ist5;
    *(unsigned long*)(Table + 19) = ist6;
    *(unsigned long*)(Table + 21) = ist7;
}





/*  设置指定的TSS  */
/*
|        reserved         |       address(63:32)        |
127                                                     64

|  address(31:24)  |    0    |  0x89   |    address(23:0)       |    limit    |
63               56  不用设置           40                      16              0
*/
static __attribute__((always_inline))
void set_tss64_descriptor(unsigned int n,void* addr)
{
    unsigned long limit = 103;  //段限长(右移16位=0)
    /*  设置低位  */
    *(unsigned long*)(GDT_Table + n) = (limit & 0xffff) | (((unsigned long)addr & 0xffffff )<< 16) | ((unsigned long)0x89 << 40) | (((unsigned long)addr >> 24 & 0xff) << 56);

    /*  设置高位  */
    *(unsigned long*)(GDT_Table +n + 1) = ((unsigned long)addr >> 32 & 0xffffffff) | 0;
}





/*  中断门描述符  */
static __attribute__((always_inline)) 
void set_intr_gate(unsigned int n,unsigned char ist,void* addr)
{
    _set_gate(IDT_Table + n, 0x8E, ist, addr);  //1000 1110
}


/*  陷进门描述符  */
static __attribute__((always_inline)) 
void set_trap_gate(unsigned int n,unsigned char ist,void* addr)
{
    _set_gate(IDT_Table + n, 0x8F, ist, addr);  //1000 1111
}
 

/* 用于系统调用的陷进门  */
static __attribute__((always_inline)) 
void set_system_gate(unsigned int n,unsigned char ist,void* addr)
{
    _set_gate(IDT_Table + n, 0xEF, ist, addr);  //1110 1111
}

/*
注：若ist参数为0则使用默认堆栈，若不为0则通过索引去TSS中找堆栈指针
*/




#endif // !__KERNEL_GATE_H
