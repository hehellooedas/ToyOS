#include <gate.h>
#include <io.h>
#include <printk.h>
#include <interrupt.h>
#include <8259A.h>
#include <lib.h>
#include <stdbool.h>

#if PIC_APIC
void do_IRQ(struct pt_regs* regs,unsigned long nr){
    irq_desc_T* irq = &interrupt_desc[nr - 32];

    if(irq->handler != NULL){
        irq->handler(nr,irq->parameter,regs);
    }
    if(irq->controler != NULL && irq->controler->ack != NULL){
        irq->controler->ack(nr);  //向中断控制器发送应答消息
    }
}
#else
void do_IRQ(unsigned long regs,unsigned long nr){
    unsigned char x;
    color_printk(RED,BLACK,"do_IRQ:%#08x\t",nr);
    x = in8(0x60);
    color_printk(RED,BLACK,"key code:%#08x\n",x);
    out8(0x20,0x20);
}
#endif




/*  发生中断时保存寄存器信息  */
#define SAVE_ALL            \
    "cld;            \n\t"   \
    "pushq %rax;     \n\t"   \
    "pushq %rax;     \n\t"   \
    "movq %es,%rax ; \n\t"   \
    "pushq %rax;     \n\t"   \
    "movq %ds,%rax;  \n\t"   \
    "pushq %rax;     \n\t"   \
    "xorq %rax,%rax; \n\t"   \
    "pushq %rbp;     \n\t"   \
    "pushq %rdi;     \n\t"   \
    "pushq %rsi;     \n\t"   \
    "pushq %rdx;     \n\t"   \
    "pushq %rcx;     \n\t"   \
    "pushq %rbx;     \n\t"   \
    "pushq %r8;      \n\t"   \
    "pushq %r9;      \n\t"   \
    "pushq %r10;     \n\t"   \
    "pushq %r11;     \n\t"   \
    "pushq %r12;     \n\t"   \
    "pushq %r13;     \n\t"   \
    "pushq %r14;     \n\t"   \
    "pushq %r15;     \n\t"   \
    "movq $0x10,%rax;\n\t"   \
    "movq %rax,%ds;  \n\t"   \
    "movq %rax,%es;  \n\t"


#define IRQ_NAME2(nr)   nr##_interrupt(void)
#define IRQ_NAME(nr)    IRQ_NAME2(IRQ##nr)

#define Build_IRQ(nr)           \
void IRQ_NAME(nr);              \
    asm (                       \
        SYMBOL_NAME_STR(IRQ)#nr"_interrupt:     \n\t"     \
        "pushq $0x00        \n\t"       \
        SAVE_ALL                        \
        "movq %rsp,%rdi     \n\t"       \
        "leaq ret_from_intr(%rip),%rax \n\t" \
        "pushq %rax         \n\t"       \
        "movq $"#nr",%rsi   \n\t"       \
        "jmp do_IRQ         \n\t"       \
    );



Build_IRQ(0x20)
Build_IRQ(0x21)
Build_IRQ(0x22)
Build_IRQ(0x23)
Build_IRQ(0x24)
Build_IRQ(0x25)
Build_IRQ(0x26)
Build_IRQ(0x27)
Build_IRQ(0x28)
Build_IRQ(0x29)
Build_IRQ(0x2a)
Build_IRQ(0x2b)
Build_IRQ(0x2c)
Build_IRQ(0x2d)
Build_IRQ(0x2e)
Build_IRQ(0x2f)
Build_IRQ(0x30)
Build_IRQ(0x31)
Build_IRQ(0x32)
Build_IRQ(0x33)
Build_IRQ(0x34)
Build_IRQ(0x35)
Build_IRQ(0x36)
Build_IRQ(0x37)


void (*interrupt[24])(void) = {
    IRQ0x20_interrupt,
    IRQ0x21_interrupt,
    IRQ0x22_interrupt,
    IRQ0x23_interrupt,
    IRQ0x24_interrupt,
    IRQ0x25_interrupt,
    IRQ0x26_interrupt,
    IRQ0x27_interrupt,
    IRQ0x28_interrupt,
    IRQ0x29_interrupt,
    IRQ0x2a_interrupt,
    IRQ0x2b_interrupt,
    IRQ0x2c_interrupt,
    IRQ0x2d_interrupt,
    IRQ0x2e_interrupt,
    IRQ0x2f_interrupt,
    IRQ0x30_interrupt,
    IRQ0x31_interrupt,
    IRQ0x32_interrupt,
    IRQ0x33_interrupt,
    IRQ0x34_interrupt,
    IRQ0x35_interrupt,
    IRQ0x36_interrupt,
    IRQ0x37_interrupt
};



void interrupt_init(void)
{
    for(int i=32;i<56;i++){
        set_intr_gate(i,2,interrupt[i - 32]);
    }
}



bool register_irq(
    unsigned long irq,
    void* arg,
    void (*handler)(unsigned long nr,unsigned long parameter,struct pt_regs* regs),
    unsigned long parameter,
    hw_int_controler* controler,
    char* irq_name
){
    irq_desc_T* p = &interrupt_desc[irq - 32];
    p->controler = controler;
    p->parameter = parameter;
    p->irq_name = irq_name;
    p->flags = 0;
    p->handler = handler;
    p->controler->installer(irq,arg);
    p->controler->enable(irq);
    return true;
}



bool unregister_irq(unsigned long irq){
    irq_desc_T* p = &interrupt_desc[irq - 32];
    p->controler->disable(irq);
    p->controler->uninstaller(irq);
    p->controler = NULL;
    p->flags = 0;
    p->irq_name = NULL;
    p->handler = NULL;
    p->parameter = 0;
    return true;
}

