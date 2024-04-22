#ifndef __DEVICE_SCREEN_H
#define __DEVICE_SCREEN_H

#include <printk.h>
#include <spinlock.h>


struct position{
    /*  当前屏幕分辨率  */
    int XResolution;  //一行有几个像素
    int YResolution;  //一列有几个像素

    /*  字符光标所在位置(坐标)  */
    int XPosition;    //当前是第几列字符
    int YPosition;    //当前是第几行字符

    /*  字符像素矩阵尺寸  */
    int XCharSize;    //每个字符要占用的列数
    int YCharSize;    //每个字符要占用的行数

    /*  鼠标光标像素矩阵尺寸  */
    int XCursorSize;
    int YCursorSize;

    /*  光标在屏幕上的坐标  */
    int XCursorPosition;    //光标所在的行数
    int YCursorPosition;    //光标所在的列数

    unsigned int pixel_cache[16][16];

    unsigned int* FB_addr;   //帧缓冲区起始地址(frame buffer)
    unsigned long FB_length; //帧缓冲区容量大小

    spinlock_T printk_lock;     //屏幕控制的自旋锁
};

extern struct position Pos;


void screen_init(void);
void frame_buffer_init(void);
void screen_clear(void);
void set_screen_blue(void);
void screen_roll_row(void);

#endif // !__DEVICE_SCREEN_H
