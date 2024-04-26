#ifndef __DSEVICE_DISK_H
#define __DSEVICE_DISK_H


#include <ptrace.h>
#include <io.h>
#include <list.h>
#include <lib.h>
#include <printk.h>
#include <semaphore.h>



extern struct block_device_operation IDE_device_operation;


/*  ATA控制命令(0X3F6)  */
#define ATA_READ    0x24    //48位LBA寻址模式读取硬盘
#define ATA_WRITE   0x34    //48位写入硬盘
#define ATA_DISK_IDENTIFY   0xEC    //硬盘设备识别信息



/*  PIO模式(I/O端口编程模式)  */
/*  磁盘0  */
#define PORT_DISK0_DATA         0x1f0  //数据端口
#define PORT_DISK0_ERROR        0x1f1  //错误状态
#define PORT_DISK0_SECTOR_CNT   0x1f2  //要操作的扇区数
#define PORT_DISK0_SECTOR_LOW   0x1f3  //扇区号/LBA(7:0)
#define PORT_DISK0_SECTOR_MID   0x1f4  //柱面号(7:0)/LBA(15:8)
#define PORT_DISK0_SECTOR_HIGH  0x1f5  //柱面号(15:8)/LBA(23:16)
#define PORT_DISK0_DEVICE       0x1f6  //设备工作方式
#define PORT_DISK0_CMD          0x1f7  //控制命令端口
#define PORT_DISK0_STATUS       0x3f6  //状态/控制寄存器



/*  磁盘1  */
#define PORT_DISK1_DATA         0x170
#define PORT_DISK1_ERROR        0x171
#define PORT_DISK1_SECTOR_CNT   0x172
#define PORT_DISK1_SECTOR_LOW   0x173
#define PORT_DISK1_SECTOR_MID   0x174
#define PORT_DISK1_SECTOR_HIGH  0x175
#define PORT_DISK1_DEVICE       0x176
#define PORT_DISK1_CMD          0x177
#define PORT_DISK1_STATUS       0x376



/*  控制器状态(0x3f6端口)  */
#define DISK_STATUS_BUSY    (1 << 7)   //磁盘在忙
#define DISK_STATUS_READY   (1 << 6)   //控制器就绪(硬盘控制器未就绪的情况下不能向它发送请求)
#define DISK_STATUS_SEEK    (1 << 4)   //正在寻道
#define DISK_STATUS_REQ     (1 << 3)   //数据请求(该位为1才能进行磁盘读写)
#define DISK_STATUS_ERROR   (1 << 0)   //命令执行错误


#define wait_when_disk0_busy()        while(in8(PORT_DISK0_STATUS) & DISK_STATUS_BUSY) nop()
#define wait_when_disk0_not_ready()   while(!(in8(PORT_DISK0_DATA) & DISK_STATUS_READY)) nop()
#define wait_when_disk0_not_req()     while(!(in8(PORT_DISK0_STATUS)& DISK_STATUS_REQ)) nop()

#define wait_when_disk1_busy()        while(in8(PORT_DISK1_STATUS) & DISK_STATUS_BUSY) nop()
#define wait_when_disk1_not_ready()   while(!(in8(PORT_DISK1_DATA) & DISK_STATUS_READY)) nop()
#define wait_when_disk1_not_req()     while(!(in8(PORT_DISK1_STATUS)& DISK_STATUS_REQ)) nop()


/*  诊断错误状态(错误端口0x1f1)  */



/*  设定硬盘的工作模式  */
#define DISK_ACCESS_MODE_CHS    0   //CHS寻址模式
#define DISK_ACCESS_MODE_LBA    1   //LBA寻址模式
#define DISK_MASTER      0   //主硬盘
#define DISK_SLAVE       1   //从硬盘
#define DISK_WORK_MODE_LBA48    0x40

/*  LBA28模式下生成硬盘设备工作模式函数(在LBA48情况下不使用)  */
static __attribute__((always_inline))
unsigned char generate_disk_work_mode(char access_mode,char hard_disk,char LBA_27_24)
{
    return (0b10100000 | (access_mode << 6) | (hard_disk << 4) | (LBA_27_24 & 0xf));
}



/*  硬盘访问链上的节点  */
struct block_buffer_node{
    unsigned int count;     //当前硬盘访问几个扇区(如果操作多个扇区,每准备好一个扇区就会发出一个中断)
    unsigned char cmd;      //硬盘访问的命令
    unsigned long LBA;      //访问硬盘的LBA地址(48)
    unsigned char* buffer;  //缓冲区地址
    void (*end_handler)(unsigned long nr,unsigned long parameter);  //回调函数
    wait_queue_T wait_queue;
};



/*  硬盘访问(请求)链  */
struct request_queue{
    wait_queue_T wait_queue_list;
    struct block_buffer_node* is_using;   //正在处理的硬盘请求
    long block_request_count;             //剩余请求数
} disk_request;



struct block_device_operation{
    long (*open)();
    long (*close)();
    long (*ioctl)(long cmd,long arg);
    long (*transfer)(long cmd,unsigned long blocks,long count,unsigned char* buffer);
} ;




/*  硬盘设备识别信息(512B)位于硬盘的ROM区域  */
struct Disk_Identify_Info{
    unsigned short general_config;      //常规配置字
    unsigned short obsolete0;
    unsigned short specific_config;     //具体配置
    unsigned short obsolete1;
    unsigned short retired0[2];
    unsigned short obsolete2;
    unsigned short compactflash[2];        //CF卡
    unsigned short retired1;
    unsigned short serial_number[10];   //序列号
    unsigned short retired2[2];
    unsigned short obsolete3;
    unsigned short firmware_version[4]; //固件版本
    unsigned short model_number[20];    //型号
    unsigned short max_logical_transreferred_per_DRQ;
    unsigned short trusted_comupting_feature_set_options;
    //用户可寻址的全部逻辑扇区数
    unsigned short capabilities0;       //Sata功能
    unsigned short capabilities1;
    unsigned short obsolete4[2];
    unsigned short report_88_70to_64_valid;
    unsigned short obsolete5[5];
    unsigned short mul_sec_setting_valid;
    unsigned short addressable_logical_sector_for_28[2];   //用户可寻址的全部逻辑扇区数
    unsigned short obsolete6;
    unsigned short multiword_DMA_select;    //
    unsigned short PIO_mode_support;
    unsigned short min_multiword_DMA_cycle_time;    //每字最小DMA传输时间
    unsigned short manufacture_recommend_mulword_DMA_cycle_time;
    unsigned short min_PIO_cycle_time_flow_control;
    unsigned short min_PIO_cycle_time_IOREADY_flow_control;
    unsigned short reserved1[2];
    unsigned short reserved2[4];
    unsigned short queue_depth;
    unsigned short SATA_capabilities;
    unsigned short reserved3;
    unsigned short SATA_features_supported;
    unsigned short SATA_features_enabled;
    unsigned short major_version;
    unsigned short minor_version;
    unsigned short cmd_features_sets_supported0;
    unsigned short cmd_features_sets_supported1;
    unsigned short cmd_features_sets_supported2;
    unsigned short cmd_features_sets_supported3;
    unsigned short cmd_features_sets_supported4;
    unsigned short cmd_features_sets_supported5;
    unsigned short ultra_DMA_modes;
    unsigned short time_required_erase_cmd;     //正常擦除模式所需时间
    unsigned short time_requeired_enhanced_cmd; //增强擦除模式所需时间
    unsigned short current_APM_level_value;     //当前APM级别(Advanced Power Management)
    unsigned short master_passowrd_identifier;
    unsigned short hardware_reset_result;
    unsigned short current_AAM_value;           //Automatic Acoustic Management管理硬盘噪音
    unsigned short stream_min_request_size;
    unsigned short streaming_transger_time_DMA;
    unsigned short streaming_access_latency_DMA_PIO;
    unsigned short streaming_performance_granularity[2];
    unsigned short addressable_logical_sector_for_48[4];    //用户可寻址的全部逻辑扇区数(48)
    unsigned short streaming_transfer_time_PIO;
    unsigned short reserved4;
    unsigned short physical_logical_sector_size;
    unsigned short inter_seek_delay;        //寻道延迟(ISO7779)
    unsigned short world_wide_name[4];      //世界通用名称
    unsigned short reserved5[4];
    unsigned short reserved6;
    unsigned short words_per_logical_sector[2];
    unsigned short cmd_features_supported;
    unsigned short cmd_features_supported_enabled;
    unsigned short reserved7[6];
    unsigned short obsolete7;
    unsigned short secturity_status;
    unsigned short vendor_specific[31];
    unsigned short CFA_power_mode;
    unsigned short reserved8[7];
    unsigned short device_from_factor;  //设备外形尺寸
    unsigned short reserved9[7];
    unsigned short current_media_serial_number[30];
    unsigned short SCT_cmd_transport;
    unsigned short reserved10[2];
    unsigned short alignment_logical_blocks_within_a_physical_block;
    unsigned short write_read_verify_sector_count_mode_3[2];
    unsigned short write_read_verify_sector_count_mode_2[2];
    /*
     * NV Cache是硬盘控制器上的一小块缓存空间
     * 硬盘在读写数据之前会临时把数据存储在这个地方
     * 数据在断电后不会丢失(容量只有几MB)
    */
    unsigned short NV_cache_capablities;
    unsigned short NV_cache_size[2];
    unsigned short nominal_media_rotation_rate; //介质转速

    unsigned short reserved11;
    unsigned short NV_cache_options;
    unsigned short write_read_verify_feature_set_current_mode;
    unsigned short reserved12;
    unsigned short transport_major_version_number;
    unsigned short transport_minor_version_number;

    unsigned short reserved13[10];
    unsigned short mini_blocks_per_cmd;
    unsigned short max_blocks_per_cmd;
    unsigned short reserved14[19];
    unsigned short integrity_word;
}__attribute__((packed));




void disk_init(void);
void disk_handler(unsigned long nr,unsigned long parameter,struct pt_regs* regs);
long IDE_open(void);
long IDE_close(void);
long IDE_ioctl(long cmd,long arg);
long IDE_transfer(long cmd,unsigned long blocks,long count,unsigned char* buffer);
long cmd_out();
struct block_buffer_node* make_request(long cmd,unsigned long blocks,long count,unsigned char* buffer);
void submit(struct block_buffer_node* node);
void wait_for_finish();
void read_handler(unsigned long nr,unsigned long parameter);
void write_handler(unsigned long nr,unsigned long parameter);
void end_request(struct block_buffer_node* node);
void other_handler();


/*  添加硬盘操作请求到队列  */
static __attribute__((always_inline))
void add_request(struct block_buffer_node* node)
{
    list_add_to_behind(&disk_request.wait_queue_list.wait_list, &node->wait_queue.wait_list);
    disk_request.block_request_count++;
}



/*  LBA28模式  */
static __attribute__((always_inline))
void Device_mode_LBA28(unsigned int count,unsigned long LBA)
{
    out8(PORT_DISK1_DEVICE,generate_disk_work_mode(1,0 ,LBA >> 24 ) );
    out8(PORT_DISK1_ERROR,0);
    out8(PORT_DISK1_SECTOR_CNT,count );
    out8(PORT_DISK1_SECTOR_LOW,LBA & 0xff );
    out8(PORT_DISK1_SECTOR_MID,(LBA >> 8) & 0xff );
    out8(PORT_DISK1_SECTOR_HIGH,(LBA >> 16) & 0xff );

}



/*  LBA48模式  */
static __attribute__((always_inline))
void Device_mode_LBA48(unsigned int count,unsigned long LBA)
{
    out8(PORT_DISK0_DEVICE,DISK_WORK_MODE_LBA48);

    out8(PORT_DISK1_ERROR,0);
    out8(PORT_DISK1_SECTOR_CNT,(count >> 8) & 0xff );
    out8(PORT_DISK1_SECTOR_LOW,(LBA >> 24) & 0xff );
    out8(PORT_DISK1_SECTOR_MID,(LBA >> 32) & 0xff );
    out8(PORT_DISK1_SECTOR_HIGH,(LBA >> 40) & 0xff );

    out8(PORT_DISK1_ERROR,0);
    out8(PORT_DISK1_SECTOR_CNT,count & 0xff );
    out8(PORT_DISK1_SECTOR_LOW,LBA & 0xff );
    out8(PORT_DISK1_SECTOR_MID,(LBA >> 8) & 0xff );
    out8(PORT_DISK1_SECTOR_HIGH,(LBA >> 16) & 0xff );
}


#endif // !__DSEVICE_DISK_H