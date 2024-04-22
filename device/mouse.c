#include <interrupt.h>
#include <mouse.h>
#include <lib.h>
#include <memory.h>
#include <APIC.h>
#include <printk.h>


struct mouse_packet mouse;
struct ioqueue* mouse_queue = NULL;
static int mouse_count = 0;

hw_int_controller mouse_int_controller = {
    .enable = IOAPIC_enbale,
    .disable = IOAPIC_disable,
    .installer = IOAPIC_install,
    .uninstaller = IOAPIC_uninstall,
    .ack = IOAPIC_edge_ack
};


void mouse_init(void)
{
    struct IO_APIC_RET_ENTRY entry;
    mouse_queue = (struct ioqueue*)kmalloc(sizeof(struct ioqueue),0);
    ioqueue_init(mouse_queue);

    entry.vector = 0x2c;
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

    register_irq(0x2c,&entry,&mouse_handler,(unsigned long)mouse_queue,&mouse_int_controller,"ps/2 mouse");

    wait_KB_write();
    out8(PORT_KB_CMD,KBCMD_EN_MOUSE_INTFACE ); //开启鼠标端口
    for(unsigned long i=0;i<1000*1000;i++){
        nop();
    }
    wait_KB_write();
    out8(PORT_KB_CMD,KBCMD_SENDTO_MOUSE );  //向鼠标设备发送数据

    wait_KB_write();

    out8(PORT_KB_DATA,MOUSE_ENABLE );
    for(unsigned long i=0;i<1000*1000;i++){
        nop();
    }

    wait_KB_write();
    out8(PORT_KB_CMD,KBCMD_WRITE_CMD );
    wait_KB_write();
    out8(PORT_KB_DATA,KB_INIT_MODE );

}




void mouse_exit(void)
{
    unregister_irq(0x2c);
    kfree((unsigned long*)mouse_queue);
}



/*  中断部分只要把从设备发来消息存储到循环队列就好  */
void mouse_handler(unsigned long irq,unsigned long parameter,struct pt_regs* regs)
{
    ioqueue_producer(mouse_queue,in8(PORT_KB_DATA));  //读取鼠标数据包,存入循环队列
}




void analysis_mousecode(void)
{
    unsigned char code = get_mousecode();
    switch (mouse_count) {
        case 0:     //跳过控制命令返回的应答数据0xfa
            mouse_count++;
            break;
        case 1:
            if((code & 0xc8) == 0x08){
                mouse.Byte0 = code;
                mouse_count++;
            }else{  //丢弃整个数据包
                return;
            }
            break;
        case 2:
            mouse.Byte1 = (char)code;
            mouse_count++;
            break;
        case 3:     //只有凑足了三个字节才是一个完整的数据包
            mouse.Byte2 = -(char)code;
            mouse_count = 1;
            color_printk(RED,BLACK,"(M:%#x,X:%3d,Y:%3d)\n",mouse.Byte0,mouse.Byte1,mouse.Byte2);
            break;
        default:
            break;
    }
}



unsigned char get_mousecode()
{
    if(mouse_queue->count == 0){  //说明循环队列里没有数据
        while (!mouse_queue->count) { //那就等到有数据进来
            pause();
        }
    }
    return ioqueue_consumer(mouse_queue);
}






void print_cursor_to_screen(unsigned int* fb){
    unsigned int* addr = NULL;
    for(int i=0;i<16;i++){   //每个字符16行
        addr = fb + Pos.XResolution * (Pos.YPosition * 16 + i) + Pos.XPosition * 16;
        for(int j=0;j<16;j++){
            Pos.pixel_cache[i][j] = *addr;  //在覆盖之前保存
            if(cursor[i][j] == '0'){
                *addr = WHITE;
            }else{
                *addr = BLACK;
            }
            addr++;
        }
    }
}