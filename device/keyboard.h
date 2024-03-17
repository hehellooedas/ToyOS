#ifndef __DEVICE_KEYBOARD_H
#define __DEVICE_KEYBOARD_H

/*  键盘驱动:以下控制器指的是键盘控制器  */

#define KB_BUF_SIZE 100  //循环队列缓冲区


struct keyboard_inputbuffer{
    unsigned char* p_head;
    unsigned char* o_tail;
    int count;
    unsigned char buf[KB_BUF_SIZE];
};


#define PORT_KB_DATA    0x60    //读写数据端口
#define PORT_KB_STATUS  0x64    //获取控制器的状态
#define PORT_KB_CMD     0x64    //向控制器发送命令

#define KBCMD_WRITE_CMD 0x60    //向键盘发送配置命令
#define KBCMD_READ_CMD  0x20    //读取键盘的配置值
#define KB_INIT_MODE    0x47    //发往键盘的配置值



#endif // !__DEVICE_KEYBOARD_H