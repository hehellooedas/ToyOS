#ifndef __DEVICE_APIC_H
#define __DEVICE_APIC_H

#include "task.h"
#include <lib.h>
#include <ptrace.h>
#include <interrupt.h>


/*
 *
 *
 *
*/



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

#endif