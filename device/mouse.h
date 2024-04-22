#ifndef __DEVICE_MOUSE_H
#define __DEVICE_MOUSE_H

#include <keyboard.h>
#include <ptrace.h>
#include <screen.h>


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
#define MOUSE_SET_DEF       0xf6    //设置默认采样率100Hz,分辨率4 pixel/mm
#define MOUSE_RESEND        0xfe    //重新发送上一条数据包
#define MOUSE_REBOOT        0xff    //重启鼠标设备




/*  3B的数据包  */
struct mouse_packet{
/* |  Y溢出  | X溢出 | Y符号位 | X符号位 | 1 | 鼠标中键 | 鼠标右键 | 鼠标左键 |
 * 如果发生了溢出,则丢弃整个数据包
 * 符号位代表了鼠标在平面直角坐标系的移动方向
 * 保留位右侧是按键状态
 */
    unsigned char Byte0;

    char Byte1;     //x轴移动值
    char Byte2;     //y轴移动值
};


extern struct mouse_packet mouse;


/*  鼠标图形  */
static const char cursor[16][16] = {
    "**************..",     //1
    "*00000000000*...",     //2
    "*0000000000*....",     //3
    "*000000000*.....",     //4
    "*00000000*......",     //5
    "*0000000*.......",     //6
    "*0000000*.......",     //7
    "*00000000*......",     //8
    "*0000**000*.....",     //9
    "*000*..*000*....",     //10
    "*00*....*000*...",     //11
    "*0*......*000*..",     //12
    "**........*000*.",     //13
    "*..........*000*",     //14
    "............*00*",     //15
    ".............***",     //16
};



/*  在指定位置打印鼠标  */



void mouse_init(void);
void mouse_handler(unsigned long irq,unsigned long parameter,struct pt_regs* regs);
unsigned char get_mousecode(void);
void analysis_mousecode(void);
void mouse_exit(void);

void print_cursor_to_screen(unsigned int* fb);
#endif