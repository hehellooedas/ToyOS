#include "io.h"
#include <disk.h>
#include <APIC.h>
#include <memory.h>



static int disk_flags = 0;  //标记当前硬盘的忙/闲状态


hw_int_controller disk_int_controller = {
    .enable = IOAPIC_enbale,
    .disable = IOAPIC_disable,
    .installer = IOAPIC_install,
    .uninstaller = IOAPIC_uninstall,
    .ack = IOAPIC_edge_ack
};



/*  这是磁盘驱动对外提供的接口(API)  */
struct block_device_operation IDE_device_operation = {
    .open = IDE_open,
    .close = IDE_close,
    .ioctl = IDE_ioctl,
    .transfer = IDE_transfer
};


void disk_init(void){

    struct IO_APIC_RET_ENTRY entry;
    entry.vector = 0x2e;
    entry.deliver_mode = APIC_DELIVER_MODE_FIXED;
    entry.dest_mode = APIC_DEST_MODE_PHY;
    entry.deliver_status = APIC_DELIVER_STATUS_IDLE;
    entry.polarity = APIC_POLARITY_HIGH;
    entry.irr = APIC_IRR_RESET;
    entry.trigger_mode = APIC_TRIGGER_MODE_EDGE;
    entry.mask = APIC_MASKED;
    entry.res_1 = 0;
    entry.destination.physical.res_2 = 0;
    entry.destination.physical.phy_dest = 0;
    entry.destination.physical.res_3 = 0;

    register_irq(0x2e,&entry,&disk_handler,(unsigned long)&disk_request,&disk_int_controller,"disk0");

    out8(PORT_DISK0_STATUS,0 );
    list_init(&disk_request.queue_list);
    disk_request.block_request_count = 0;
    disk_request.is_using = NULL;
}



void disk_handler(unsigned long nr,unsigned long parameter,struct pt_regs* regs){
    struct block_buffer_node* node = ((struct request_queue*)parameter)->is_using;
    node->end_handler(nr,parameter);
}



void disk_exit()
{
    unregister_irq(0x2e);
}



long IDE_open(void)
{
    color_printk(BLACK,WHITE ,"disk has opened!\n" );
    return 1;
}



long IDE_close(void)
{
    color_printk(BLACK,WHITE ,"disk has closed!\n" );
    return 1;
}


long IDE_ioctl(long cmd,long arg)
{
    struct block_buffer_node* node = NULL;
    if(cmd == ATA_DISK_IDENTIFY){
        node = make_request(cmd,0 ,0 ,(unsigned char*)arg );
        submit(node);
        wait_for_finish();
        return 1;
    }
    return 0;
}



/*  这是硬盘读写的入口  */
long IDE_transfer(long cmd,unsigned long blocks,long count,unsigned char* buffer)
{
    struct block_buffer_node* node = NULL;
    if(cmd == ATA_READ || cmd == ATA_WRITE){
        node = make_request(cmd,blocks,count,buffer);
        submit(node);   //把制作好的请求链添加到disk_request上面去
        wait_for_finish();  //如果当前磁盘空闲,在submit()就直接执行,否则等待
    }else{
        return 0;
    }
    return 1;
}



/*  排队轮到之后向硬盘发送要读/写的消息  */
long cmd_out(){
    struct block_buffer_node* node = disk_request.is_using = container_of(get_List_next(&disk_request.queue_list),struct block_buffer_node ,list );
    list_del(&disk_request.is_using->list);
    disk_request.block_request_count--;
    wait_when_disk0_busy();
    switch (node->cmd) {
        case ATA_READ:
            Device_mode_LBA48(node->count,node->LBA );
            wait_when_disk0_not_ready();
            out8(PORT_DISK0_CMD,node->cmd);     //向硬盘发送读请求(硬盘把数据放置到NV Cache后会发中断过来,有一个准备数据和触发中断的过程)
            break;
        case ATA_WRITE:
            Device_mode_LBA48(node->count,node->LBA );
            wait_when_disk0_busy();
            out8(PORT_DISK0_CMD,node->cmd );

            wait_when_disk0_not_req();
            /*  而写操作硬盘和读硬盘不同,不需要硬盘发来中断才能写,数据请求位允许之后就能直接写  */
            outsw(PORT_DISK0_DATA,node->buffer,256);
            break;
        case ATA_DISK_IDENTIFY:
            /*  获取硬盘信息使用的是LBA28模式  */
            out8(PORT_DISK0_DEVICE,0xe0);

            out8(PORT_DISK0_ERROR,0 );
            out8(PORT_DISK0_SECTOR_CNT,node->count & 0xff );
            out8(PORT_DISK0_SECTOR_LOW,node->LBA & 0xff );
            out8(PORT_DISK0_SECTOR_MID,(node->LBA >> 8) & 0xff );
            out8(PORT_DISK0_SECTOR_HIGH,(node->LBA >> 16) & 0xff );
            wait_when_disk0_not_ready();

            out8(PORT_DISK0_CMD,node->cmd );
            break;
        default:
            color_printk(RED,BLACK ,"current ATA command %#x is not found!\n",node->cmd );
            break;
    }

    return 1;
}



/*  制作硬盘操作的请求链  */
struct block_buffer_node* make_request(long cmd,unsigned long blocks,long count,unsigned char* buffer)
{
    struct block_buffer_node* node = (struct block_buffer_node*)kmalloc(sizeof(struct block_buffer_node), 0);
    list_init(&node->list);
    switch (cmd) {
        case ATA_READ:
            node->cmd = ATA_READ;
            node->end_handler = read_handler;
            break;
        case ATA_WRITE:
            node->cmd = ATA_WRITE;
            node->end_handler = write_handler;
            break;
        default:
            node->cmd = 0xff;
            node->end_handler = other_handler;
            break;
    }
    node->LBA = blocks;
    node->count = count;
    node->buffer = buffer;
    return node;
}


/*  提交硬盘操作请求  */
void submit(struct block_buffer_node* node)
{
    add_request(node);
    if(disk_request.is_using == NULL){  //当前硬盘空闲
        cmd_out();
    }
}


/*  循环等待直到硬盘空闲  */
void wait_for_finish()
{
    disk_flags = 1;
    while (disk_flags) {
        nop();
    }
}



/*  读硬盘的回调函数(发送要读的消息后)  */
void read_handler(unsigned long nr,unsigned long parameter){
    struct block_buffer_node* node = disk_request.is_using;
    if(in8(PORT_DISK0_STATUS) & DISK_STATUS_ERROR){  //磁盘控制器发来错误信号
        color_printk(RED,BLACK ,"read_handler Error:%#x",in8(PORT_DISK0_ERROR) );
    }else{
        insw(PORT_DISK0_DATA,node->buffer,256);
    }
    end_request();
}



void write_handler(unsigned long nr,unsigned long parameter){
    if(in8(PORT_DISK0_STATUS) & DISK_STATUS_ERROR){  //磁盘控制器发来错误信号
        color_printk(RED,BLACK ,"write_handler:#%x",in8(PORT_DISK0_ERROR) );
    }
    end_request();
}



void end_request(){
    kfree((unsigned long *)disk_request.is_using);
    disk_request.is_using = NULL;
    disk_flags = 0;     //磁盘空闲了
    if(disk_request.block_request_count){   //队列里还有任务则继续执行
        cmd_out();
    }
}


void other_handler(){
    color_printk(RED,BLACK ,"Sorry to tell you the dirver accept error cmd!\n" );
}
