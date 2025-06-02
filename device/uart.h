#ifndef __DEVICE_UART_H
#define __DEVICE_UART_H

#include <io.h>
#include <lib.h>
#include <printk.h>

/*
  串行通信端口
  16550 UART串口控制器使用以下标准I/O端口
  偏移量    端口地址        寄存器名称          作用
  +0        0x3F8       数据寄存器            发送/接收数据
  +1        0x3F9       中断使能寄存器(IER)       控制哪些中断被启用
  +2        0x3FA       中断标识寄存器(IIR)       识别中断源
  +3        0x3FB       线路控制寄存器(LCR)       设置数据格式
  +4        0x3FC       Modem控制寄存器       控制调制解调器信号线
  +5        0x3FD       线路状态寄存器(LSR)       查看发送/接收状态
  +6        0x3FE       Modem状态寄存器(MCR)     查看调制解调器信号线状态
  +7        0x3FF       暂存寄存器            通用存储寄存器
*/

/*
 * 波特率计算公式:波特率 = 基准时钟频率 / (除数 * 16)
 * 在UART通讯中，发送方和接收方都需要按照相同的波特率(baud rate)进行工作,同时UART芯片需要一个稳定的时钟才能保证数据得到正确传输
 * 常用波特率:9600,19200,38400,57600,115200
 * 常见基准时钟频率:1.8432MHz
 * 115200对应除数1,38400对应除数3,9600对应除数0xC
*/





/*  端口号  */
#define COM1_PORT      0x3F8    //COM1的基址
#define COM2_PORT      0x2F8
#define COM3_PORT      0x3E8
#define COM4_PORT      0x2E8
#define COM5_PORT      0x5F8
#define COM6_PORT      0x4F8
#define COM7_PORT      0x5E8
#define COM8_PORT      0x4E8



static __attribute__((always_inline))
unsigned long serial_init(void){
    out8(COM1_PORT + 1,0x00);   //禁用中断
    out8(COM1_PORT + 3, 0x80);  //启用DLAB(设置波特率除数)
    out8(COM1_PORT + 0, 0x03);  //设置除数3(低8位)
    out8(COM1_PORT + 1, 0x00);  //高8位
    out8(COM1_PORT + 3, 0x03);  //8位数据，无奇偶校验，1停止位
    out8(COM1_PORT + 2, 0xc7);  //启用FIFO，清空，14字节阈值
    out8(COM1_PORT + 4, 0x0B);  //启用IRQ，设置RTS/DSR
    return 0;
}



#endif // !__DEVICE_UART_H