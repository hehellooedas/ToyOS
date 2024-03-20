#ifndef __DEVICE_APIC_H
#define __DEVICE_APIC_H


#include <lib.h>
#include <ptrace.h>
#include <interrupt.h>


/*
 *
 *
 *
*/


/*  LVT结构(只有定时器有timer_mode)  */
struct APIC_LVT{
    unsigned int vector:8,          //中断向量号
                 deliver_mode:3,    //投递模式
                 res_1:1,           //保留
                 deliver_status:1,  //投递状态
                 polarity:1,        //电平触发极性
                 irr:1,             //远程IRR标志位
                 trigger_mode:1,    //触发模式
                 mask:1,            //屏蔽标志位
                 timer_mode:2,      //定时模式
                 res_2:13;          //保留
}__attribute__((packed));           //位字段方便按位赋予功能


/*  APIC的投递模式表(deliver_mode)  */
#define APIC_DELIVER_MODE_FIXED     0b000   //由LVT寄存器的向量号区域指定中断向量号
#define APIC_DELIVER_MODE_SMI       0b010   //通过处理器的SMI信号线向处理器投递SMI(向量号区域必须为0)
#define APIC_DELIVER_MODE_NMI       0b100   //向处理器投递NMI请求(忽略向量号)
#define APIC_DELIVER_MODE_INT       0b101
#define APIC_DELIVER_MODE_ExtINT    0b111   //接受类8259A的中断请求


/*  目标模式(仅RTE使用)  */
#define APIC_DEST_MODE_PHY  0       //物理目标模式(使用APIC ID号来确定接收中断消息的处理器)
#define APIC_DEST_MODE_LOGIC        //逻辑目标模式(使用LDR和DFR寄存器提供的自定义ID号~)


/*  APIC的投递状态(deliver_status)  */
#define APIC_DELIVER_STATUS_IDLE    0       //目前中断源没有或投递给处理器后已受理
#define APIC_DELIVER_STATUS_SEND_PENDING    1   //投递了但是未受理


/*  电平触发极性  */
#define APIC_POLARITY_HIGH      0   //高电平触发
#define APIC_POLARITY_LOW       1   //低电平触发


/*  远程IRR(只对Fix有效)  */
#define APIC_IRR_RESET      0
#define APIC_IRR_ACCEPT     1


/*  触发模式  */
#define APIC_TRIGGER_MODE_EDGE  0   //边缘触发
#define APIC_TRIGGER_MODE_LEVEL 1   //电平触发


/*  屏蔽标志位  */
#define APIC_UN_MASKED         0    //未屏蔽
#define APIC_MASKED            1    //已屏蔽


/*  定时模式(0b11为保留)  */
#define APIC_TIMER_ONE_SHOT        0b00   //一次性定时
#define APIC_TIMER_PERIODIC        0b01   //周期性定时
#define APIC_TIMER_TSC_DEADLINE    0b10   //指定TSC值计数


/*---------------------------------------------*/


/*  I/O中断定向投递寄存器组(它和LVT长得很像)  */
struct IO_APIC_RET_ENTRY{
    unsigned int vector:8,
                 deliver_mode:3,
                 dest_mode:1,
                 deliver_status:1,
                 polarity:1,
                 irr:1,
                 trigger_mode:1,
                 mask:1,
                 res_1:15;
    union{
        struct {
            unsigned int res_2:24,
                         phy_dest:4,  //用来保存APIC ID表示接收者
                         res_3:4;
        }physical;
        struct {
            unsigned int res_2:24,
                         logical_dest:8;    //自定义APIC ID号
        }logical;
    }destination;
}__attribute__((packed));





struct IOAPIC_map{
    unsigned int physical_address;        //间接访问寄存器的物理基地址
    unsigned char* virtual_index_address; //索引寄存器(IOREGSEL)
    unsigned int* virtual_data_address;   //数据寄存器(IOWIN)
    unsigned int* virtual_EOI_address;    //EOI寄存器
}ioapic_map;  //需要一个全局变量来存储I/O APIC的"寄存器"





static __attribute__((always_inline))
unsigned int ioapic_read(unsigned char index)
{
    *ioapic_map.virtual_index_address = index;
    lfence();
    return (unsigned int)*ioapic_map.virtual_data_address;
}


static __attribute__((always_inline))
void ioapic_write(unsigned char index,unsigned int value)
{
    *ioapic_map.virtual_index_address = index;
    mfence();
    *ioapic_map.virtual_data_address = value;
    mfence();
}




/*  RTE类寄存器为64字节读写分别需要两次  */
static __attribute__((always_inline))
unsigned long ioapic_rte_read(unsigned char index)
{
    unsigned long ret;
    *ioapic_map.virtual_index_address = index + 1;
    lfence();
    ret = *ioapic_map.virtual_data_address;
    ret <<= 32;
    mfence();

    *ioapic_map.virtual_index_address = index;
    lfence();
    ret |= *ioapic_map.virtual_data_address;
    mfence();

    return ret;
}



static __attribute__((always_inline))
void ioapic_rte_write(unsigned char index,unsigned long value)
{
    *ioapic_map.virtual_index_address = index;
    mfence();
    *ioapic_map.virtual_data_address = value & 0xffffffff;
    value >>= 32;
    mfence();

    *ioapic_map.virtual_index_address = index + 1;
    mfence();
    *ioapic_map.virtual_data_address = value & 0xffffffff;
    mfence();
}



void Local_APIC_init(void);
void IOAPIC_init(void);
void APIC_IOAPIC_init(void);
void IOAPIC_enbale(unsigned long irq);
void IOAPIC_disable(unsigned long irq);
bool IOAPIC_install(unsigned long irq,void* arg);
void IOAPIC_uninstall(unsigned long irq);
void IOAPIC_level_ack(unsigned long irq);
void IOAPIC_edge_ack(unsigned long irq);

#endif