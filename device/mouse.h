#ifndef __DEVICE_MOUSE_H
#define __DEVICE_MOUSE_H

#include <keyboard.h>

/*
 * PS/2接口的鼠标接口归键盘控制器管理,降低额外电路成本
 * 一些老式数码笔、触摸屏控制器、手柄也依赖于PS/2接口
*/


#define KBCMD_SENDTO_MOUSE  0xd4    //向鼠标设备发送数据
#define MOUSE_ENABLE        0xf4    //允许鼠标发送数据包

#define KBCMD_EN_MOUSE_INTFACE  0xa8    //开启鼠标端口


struct mouse_packet{
    unsigned char Byte0;

    char Byte1;     //x轴坐标
    char Byte2;     //y轴坐标
};


void mouse_init(void);

#endif