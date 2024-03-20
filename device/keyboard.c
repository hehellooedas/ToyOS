#include <interrupt.h>
#include <keyboard.h>
#include <lib.h>
#include <memory.h>
#include <APIC.h>
#include <printk.h>


struct ioqueue* keyboard_queue = NULL;
static int shift_l,shift_r,ctrl_l,ctrl_r,alt_l,alt_r; //几个重要的控制按键


hw_int_controler keyboard_int_controler = {
    .enable = IOAPIC_enbale,
    .disable = IOAPIC_disable,
    .installer = IOAPIC_install,
    .uninstaller = IOAPIC_uninstall,
    .ack = IOAPIC_edge_ack
};


void keyboard_init(void)
{
    struct IO_APIC_RET_ENTRY entry;
    keyboard_queue = (struct ioqueue*)kmalloc(sizeof(struct ioqueue),0 );
    ioqueue_init(keyboard_queue);


    entry.vector = 0x21;
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

    wait_KB_write();
    out8(PORT_KB_CMD,KBCMD_WRITE_CMD);
    wait_KB_read();
    out8(PORT_KB_DATA,KB_INIT_MODE );

    for(unsigned long i=0;i<1000*1000;i++){
        nop();      //让低速的键盘控制器把控制命令执行完
    }

    shift_l = shift_r = ctrl_l = ctrl_r = alt_l = alt_r = 0;
    register_irq(0x21,&entry,&keyboard_handler,(unsigned long)keyboard_queue,&keyboard_int_controler,"ps/2 keyboard");
}



/*  驱动卸载程序  */
void keyboard_exit(void)
{
    unregister_irq(0x21);
    kfree(keyboard_queue);
}



void keyboard_handler(unsigned long nr,unsigned long parameter,struct pt_regs* regs)
{
    ioqueue_producer(keyboard_queue,in8(0x60)); //读取键盘扫描码
}



void analysis_keycode()
{
    int key=0,make=0;
    unsigned char x = get_scancode();
    if(x == 0xE1){  //第一类键盘扫描码
        key = PULASEBREAK;
        for(int i=0;i<6;i++){   //循环判断是否为pulse按键
            if(get_scancode() != pulsebreak_scode[i]){
                key = 0;
                break;
            }
        }
    }else if (x == 0xE0) {  //如果不是第一类则判断是否为第二类特殊按键
        x = get_scancode();
        switch (x) {
            case 0x2A:  //Print/Scan
                if(get_scancode() == 0xE0){
                    if(get_scancode() == 0x37){
                        key = PRINTSCREEN;
                        make = 1;
                    }
                }
                break;
            case 0xB7:  //松开了Print/Scan
                if(get_scancode() == 0xE0){
                    if(get_scancode() == 0xAA){
                        key = PRINTSCREEN;
                        make = 0;
                    }
                }
                break;
            case 0x1D:  //右Ctrl键
                if(get_scancode() == 0x1D){
                    key = OTHERKEY;
                    ctrl_r = 1;
                    make = 1;
                }
                break;
            case 0x9D:  //松开了右Ctrl
                if(get_scancode() == 0x1D){
                    key = OTHERKEY;
                    ctrl_r = 1;
                    make = 0;
                }
                break;
            case 0x38:  //右Alt键
                key = OTHERKEY;
                alt_r = 1;
                make = 1;
                break;
            case 0xB8:  //松开了右ALt
                key = OTHERKEY;
                alt_r = 1;
                make = 0;
                break;
            default:
                key = OTHERKEY;
                break;
        }
    }
    /*  第三类是普通按键,不需要单独判断  */
    if(key == 0){   //key为0不是第一类与第二类
        unsigned int* keyrow = NULL;
        int column = 0;
        make = (x & FLAG_BREAK ? 0:1);
        keyrow = &keycode_map_normal[(x & 0x7f) * MAP_COLS];
        if(shift_l | shift_r){  //进行Shift化(变化方式存储在keycode_map_normal里)
            column = 1;
        }
        key = keyrow[column];
        switch (x & 0x7f) {
            case 0x2A:      //左Shift
                shift_l = make;
                key = 0;
                break;
            case 0x36:      //右Shift
                shift_r = make;
                key = 0;
                break;
            case 0x1d:      //左Ctrl
                ctrl_l = make;
                key = 0;
                break;
            case 0x38:      //左Alt
                alt_l = make;
                key = 0;
                break;
            default:
                if(!make){
                    key = 0;
                }
                break;
        }
    }

    if(key){
        color_printk(RED,BLACK ,"%c",key );
    }
}



unsigned char get_scancode()
{
    if(keyboard_queue->count == 0){  //说明循环队列里没有数据
        while (!keyboard_queue->count) { //那就等到有数据进来
            nop();
        }
    }

    return ioqueue_consumer(keyboard_queue);
}