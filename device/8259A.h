#ifndef __DEVICE_8259A_H
#define __DEVICE_8259A_H

#include <io.h>


struct IC_8259A{
    unsigned char ctrl_port;  //控制端口
    unsigned char data_port;  //数据端口

    unsigned char ICW1;  //初始化芯片的连接方式和触发方式
    unsigned char ICW2;  //初始化该芯片接管的起始中断向良号
    unsigned char ICW3;  //初始化级联的具体方式
    unsigned char ICW4;  //设置中断管理的模式

    unsigned char OCW1;  //设置屏蔽的中断信号(放行/屏蔽)
};



static __attribute__((always_inline))
struct IC_8259A create_8259A(int index)
{
    if(index){
        struct IC_8259A slave_8259A = {
            .ctrl_port = 0xa0,
            .data_port = 0xa1,
            .ICW1 = 0x11,
            .ICW2 = 0x28,
            .ICW3 = 0x02,
            .ICW4 = 0x01,
            .OCW1 = 0xff
        };
        return slave_8259A;
    }else{
        struct IC_8259A master_8259A = {
            .ctrl_port = 0x20,
            .data_port = 0x21,
            .ICW1 = 0x11,
            .ICW2 = 0x20,
            .ICW3 = 0x04,
            .ICW4 = 0x01,
            .OCW1 = 0xff
        };
        return master_8259A;
    }
}


static __attribute__((always_inline))
void pic_init(struct IC_8259A pic)
{
    out8(pic.ctrl_port,pic.ICW1);
    out8(pic.data_port,pic.ICW2);
    out8(pic.data_port,pic.ICW3);
    out8(pic.data_port,pic.ICW4);
    out8(pic.data_port,pic.OCW1);
}



#define master_timer        0x01    //时钟中断
#define master_keyboard     0x02    //键盘中断
#define master_link         0x04    //级联接口
#define master_serial_2     0x08    //串行口2
#define master_serial_1     0x10    //串行口1
#define master_parallel_2   0x20    //并行口2
#define master_floppy_disk  0x40    //软盘
#define master_parallel_1   0x80    //并行口1

#define slave_RTC           0x01    //实时钟
#define slave_link          0x02    //重定向到master_link
#define slave_PS2           0x10    //鼠标中断
#define slave_coprocesser   0x20    //协处理器
#define slave_master_disk   0x40    //主硬盘
#define slave_slave_disk    0x80    //从硬盘


static __attribute__((always_inline))
void pic_enable_interrupt(struct IC_8259A pic,unsigned char x)
{
    pic.OCW1 &= ~x;
    out8(pic.data_port,pic.OCW1);
}



static __attribute__((always_inline))
void pic_disable_interrupt(struct IC_8259A pic,unsigned char x)
{
    pic.OCW1 |= x;
    out8(pic.data_port,pic.OCW1);
}


#endif // !__DEVICE_8259A_H
