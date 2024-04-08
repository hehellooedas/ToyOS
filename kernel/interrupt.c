#include <task.h>
#include <SMP.h>
#include <gate.h>
#include <io.h>
#include <printk.h>
#include <interrupt.h>
#include <8259A.h>
#include <lib.h>
#include <stdbool.h>
#include <APIC.h>
#include <schedule.h>



#if PIC_APIC
void do_IRQ(struct pt_regs* regs,unsigned long nr){
    switch (nr & 0x80) {
        case 0x00:
            irq_desc_T* irq = &interrupt_desc[nr - 32];

            if(irq->handler != NULL){
                irq->handler(nr,irq->parameter,regs);
            }
            if(irq->controller != NULL && irq->controller->ack != NULL){
                irq->controller->ack(nr);  //向中断控制器发送应答消息
            }
            break;
        case 0x80:
            //color_printk(YELLOW,BLACK ,"currect->thread->rsp0=%#lx\n",current->thread->rsp0 );
            Local_APIC_edge_level_ack(nr);
            {
                irq_desc_T* irq = &SMP_IPI_desc[nr - 200];
                if(irq->handler != NULL){
                    irq->handler(nr,irq->parameter,regs);
                }
            }
            break;
        default:
            color_printk(RED,BLACK ,"do_IRQ receive:%d\n",nr );
            break;
    }
}
#else
void do_IRQ(struct pt_regs* regs,unsigned long nr){
    switch (nr & 0x80) {
        case 0x00:
            unsigned char x;
            color_printk(RED,BLACK,"do_IRQ:%#08x\t",nr);
            x = in8(0x60);
            color_printk(RED,BLACK,"key code:%#08x\n",x);
            out8(0x20,0x20);
            break;
        case 0x80:
            color_printk(RED,BLACK ,"SMP IPI:%d\n",nr );
            Local_APIC_edge_level_ack(nr);
            break;
        default:
            color_printk(RED,BLACK ,"do_IRQ receive:%d\n",nr );
            break;
    }
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


/*  IPI消息的中断向量  */
Build_IRQ(0xc8)
Build_IRQ(0xc9)
Build_IRQ(0xca)
Build_IRQ(0xcb)
Build_IRQ(0xcc)
Build_IRQ(0xcd)
Build_IRQ(0xce)
Build_IRQ(0xcf)
Build_IRQ(0xd0)
Build_IRQ(0xd1)


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



void (*SMP_interrupt[10])(void) = {
    IRQ0xc8_interrupt,
    IRQ0xc9_interrupt,
    IRQ0xca_interrupt,
    IRQ0xcb_interrupt,
    IRQ0xcc_interrupt,
    IRQ0xcd_interrupt,
    IRQ0xce_interrupt,
    IRQ0xcf_interrupt,
    IRQ0xd0_interrupt,
    IRQ0xd1_interrupt,
};

irq_desc_T interrupt_desc[NR_IRQS] = {0};
irq_desc_T SMP_IPI_desc[10] = {0};



void interrupt_init(void)
{
    for(int i=32;i<56;i++){
        set_intr_gate(i,0,interrupt[i - 32]);
    }
}



bool register_irq(
    unsigned long irq,
    void* arg,
    void (*handler)(unsigned long nr,unsigned long parameter,struct pt_regs* regs),
    unsigned long parameter,
    hw_int_controller* controller,
    char* irq_name
){
    irq_desc_T* p = &interrupt_desc[irq - 32];
    p->controller = controller;
    p->parameter = parameter;
    p->irq_name = irq_name;
    p->flags = 0;
    p->handler = handler;
    p->controller->installer(irq,arg);
    p->controller->enable(irq);
    return true;
}



bool unregister_irq(unsigned long irq){
    irq_desc_T* p = &interrupt_desc[irq - 32];
    p->controller->disable(irq);
    p->controller->uninstaller(irq);
    p->controller = NULL;
    p->flags = 0;
    p->irq_name = NULL;
    p->handler = NULL;
    p->parameter = 0;
    return true;
}



bool register_IPI(unsigned long irq,
                  void* arg,
                  void (*handler)(unsigned long nr,unsigned long parameter,struct pt_regs* regs),
                  unsigned long parameter,
                  hw_int_controller* controller,
                  char* irq_name
){
    irq_desc_T* p = &SMP_IPI_desc[irq - 200];
    p->controller = NULL;
    p->irq_name = irq_name;
    p->parameter = parameter;
    p->flags = 0;
    p->handler = handler;
    return 1;
}

