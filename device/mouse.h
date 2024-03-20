#ifndef __DEVICE_MOUSE_H
#define __DEVICE_MOUSE_H

#include <keyboard.h>
#include <ptrace.h>

/*
 * PS/2接口的鼠标接口归键盘控制器管理,降低额外电路成本
 * 一些老式数码笔、触摸屏控制器、手柄也依赖于PS/2接口
*/


#define KBCMD_SENDTO_MOUSE  0xd4    //向鼠标设备发送数据

#define KBCMD_EN_MOUSE_INTFACE  0xa8    //开启鼠标端口
#define KBCMD_DIS_MOUSE_INTFACE 0xa7    //禁止鼠标端口


#define MOUSE_GET_ID        0xf2    //获取鼠标设备的ID号
#define MOUSE_SET_RATE      0xf3    //设置鼠标采样率
#define MOUSE_ENABLE        0xf4    //允许鼠标发送数据包
#define MOUSE_DISABLE       0xf5    //禁止鼠标发送数据包
#define MOUSE_SET_DEF       0xf6
#define MOUSE_RESEND        0xfe    //重新发送上一条数据包
#define MOUSE_REBOOT        0xff    //重启鼠标设备



/*  3B的数据包  */
struct mouse_packet{
    unsigned char Byte0;

    char Byte1;     //x轴坐标
    char Byte2;     //y轴坐标
}mouse;


void mouse_init(void);
void mouse_handler(unsigned long irq,unsigned long parameter,struct pt_regs* regs);
unsigned char get_mousecode(void);
void analysis_mousecode(void);
void mouse_exit(void);

#endif