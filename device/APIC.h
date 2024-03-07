#ifndef __DEVICE_APIC_H
#define __DEVICE_APIC_H


struct IOAPIC_map{
    unsigned int physical_address; //间接访问寄存器的物理基地址
    unsigned char* virtual_index_address; //索引寄存器
    unsigned int* virtual_data_address;   //数据寄存器
    unsigned int* virtual_EOI_address;    //EOI寄存器
};


#endif