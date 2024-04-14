#include <io.h>
#include <lib.h>
#include <list.h>
#include <printk.h>
#include <disk.h>
#include <APIC.h>
#include <memory.h>





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
    entry.vector = 0x2f;
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

    register_irq(0x2f,&entry,&disk_handler,(unsigned long)&disk_request,&disk_int_controller,"disk1");

    out8(PORT_DISK1_STATUS,0 );
    wait_queue_init(&disk_request.wait_queue_list,NULL);
    disk_request.block_request_count = 0;
    disk_request.is_using = NULL;
}



void disk_handler(unsigned long nr,unsigned long parameter,struct pt_regs* regs){
    struct block_buffer_node* node = ((struct request_queue*)parameter)->is_using;
    node->end_handler(nr,parameter);
}



void disk_exit()
{
    unregister_irq(0x2f);
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
        submit(node);       //把制作好的请求链添加到disk_request上面去
        wait_for_finish();  //如果当前磁盘空闲,在submit()就直接执行,否则等待(会失去CPU)
    }else{
        return 0;
    }
    return 1;
}



/*  排队轮到之后向硬盘发送要读/写的消息  */
long cmd_out(){
    wait_queue_T* wait_queue_tmp = container_of(get_List_next(&disk_request.wait_queue_list.wait_list),wait_queue_T,wait_list);
    struct block_buffer_node* node = disk_request.is_using = container_of(wait_queue_tmp,struct block_buffer_node ,wait_queue);

    list_del(&disk_request.is_using->wait_queue.wait_list);
    disk_request.block_request_count--;
    wait_when_disk1_busy();
    switch (node->cmd) {
        case ATA_READ:
            Device_mode_LBA48(node->count,node->LBA );
            wait_when_disk1_not_ready();
            out8(PORT_DISK1_CMD,node->cmd);     //向硬盘发送读请求(硬盘把数据放置到NV Cache后会发中断过来,有一个准备数据和触发中断的过程)
            break;
        case ATA_WRITE:
            Device_mode_LBA48(node->count,node->LBA );
            wait_when_disk1_busy();
            out8(PORT_DISK1_CMD,node->cmd );

            wait_when_disk1_not_req();
            /*  而写操作硬盘和读硬盘不同,不需要硬盘发来中断才能写,数据请求位允许之后就能直接写  */
            outsw(PORT_DISK1_DATA,node->buffer,256);
            break;
        case ATA_DISK_IDENTIFY:
            /*  获取硬盘信息使用的是LBA28模式  */
            out8(PORT_DISK1_DEVICE,0xe0);

            out8(PORT_DISK1_ERROR,0 );
            out8(PORT_DISK1_SECTOR_CNT,node->count & 0xff );
            out8(PORT_DISK1_SECTOR_LOW,node->LBA & 0xff );
            out8(PORT_DISK1_SECTOR_MID,(node->LBA >> 8) & 0xff );
            out8(PORT_DISK1_SECTOR_HIGH,(node->LBA >> 16) & 0xff );
            wait_when_disk1_not_ready();

            out8(PORT_DISK1_CMD,node->cmd );
            break;
        default:
            color_printk(RED,BLACK ,"current ATA command %#x is not found!\n",node->cmd );
            break;
    }

    return 1;
}



/*  制作硬盘操作的请求链  */
struct block_buffer_node* make_request
(long cmd,unsigned long blocks,long count,unsigned char* buffer)
{
    struct block_buffer_node* node = (struct block_buffer_node*)kmalloc(sizeof(struct block_buffer_node), 0);
    wait_queue_init(&node->wait_queue,current);
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


/*   让当前进程去休息,因为正在等待硬盘  */
void wait_for_finish()
{
    current->state = TASK_UNINTERRUPTIBLE;
    schedule();
}



/*  读硬盘的回调函数(发送要读的消息后)  */
void read_handler(unsigned long nr,unsigned long parameter){
    struct block_buffer_node* node = ((struct request_queue*)parameter)->is_using;
    if(in8(PORT_DISK1_STATUS) & DISK_STATUS_ERROR){  //磁盘控制器发来错误信号
        color_printk(RED,BLACK ,"read_handler Error:%#x",in8(PORT_DISK1_ERROR) );
    }else{
        insw(PORT_DISK1_DATA,node->buffer,256);
    }
    node->count--;
    if(node->count){
        node->buffer += 512;
        return;
    }
    end_request(node);
}



void write_handler(unsigned long nr,unsigned long parameter){
    struct block_buffer_node* node = ((struct request_queue*)parameter)->is_using;
    if(in8(PORT_DISK1_STATUS) & DISK_STATUS_ERROR){  //磁盘控制器发来错误信号
        color_printk(RED,BLACK ,"write_handler:#%x",in8(PORT_DISK1_ERROR) );
    }
    node->count--;
    if(node->count){
        node->buffer += 512;
        wait_when_disk1_not_req();
        outsw(PORT_DISK1_DATA,node->buffer,256);
        return;
    }
    end_request(node);
}



/*  硬盘操作请求结束后的善后工作  */
void end_request(struct block_buffer_node* node){
    if(node == NULL) {
        color_printk(RED,BLACK ,"end_request error\n" );
    }
    node->wait_queue.task->state = TASK_RUNNING;
    insert_task_queue(node->wait_queue.task);
    node->wait_queue.task->flags |= NEED_SCHEDULE;  //从中断返回的时候就会schedule()

    kfree((unsigned long *)disk_request.is_using);
    disk_request.is_using = NULL;

    if(disk_request.block_request_count){   //队列里还有任务则继续执行
        cmd_out();
    }
    //sti();
}


void other_handler(){
    color_printk(RED,BLACK ,"Sorry to tell you the dirver accept error cmd!\n" );
}
