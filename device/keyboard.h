#ifndef __DEVICE_KEYBOARD_H
#define __DEVICE_KEYBOARD_H


#include <ptrace.h>
#include <stdbool.h>
#include <io.h>
#include <ioqueue.h>

/*
 * 键盘驱动(PS/2控制器)是高级集成外设的一部分
 * 以下控制器指的是键盘控制器
 * PS/2接口是围绕着Intel8042芯片设计和定义的
 * 类8048键盘编码器芯片不停地扫描键盘的每一个按键
 * 现代硬件设备逐渐替换成USB接口和南桥芯片组
*/


#define KEY_CMD_RESET_BUFFER    (1 << 0)        //清空键盘缓冲区命令


#define PORT_KB_DATA    0x60    //读写数据端口
#define PORT_KB_STATUS  0x64    //获取控制器的状态
#define PORT_KB_CMD     0x64    //向控制器发送命令

#define KBCMD_WRITE_CMD 0x60    //向键盘发送配置命令
#define KBCMD_READ_CMD  0x20    //读取键盘的配置值
#define KB_INIT_MODE    0x47    //发往键盘的配置值

#define KBSTATUS_IBF    0x02
#define KBSTATUS_OBF    0x01

#define wait_KB_write() while(in8(PORT_KB_STATUS) & KBSTATUS_IBF)
#define wait_KB_read()  while(in8(PORT_KB_STATUS) & KBSTATUS_OBF)


#define NR_SCAN_CODES   0x80    //128个按键
#define MAP_COLS        2       //每个按键有两种状态

#define PULASEBREAK     1       //
#define PRINTSCREEN     2
#define OTHERKEY        4
#define FLAG_BREAK      0x80


unsigned char pulsebreak_scode[] = {0xE1,0x1D,0x45,0xE1,0x9D,0xC5};  //第一类键盘扫描码(pulse键的键码是真的长)


unsigned int keycode_map_normal[NR_SCAN_CODES * MAP_COLS] = {
/*      scan-code       unShift         Shift   */
        /*0x00*/           0,             0,
        /*0x01*/           0,             0,    //ESC
        /*0x02*/           1,            '!',
        /*0x03*/           2,            '@',
        /*0x04*/           3,            '#',
        /*0x05*/           4,            '$',
        /*0x06*/           5,            '%',
        /*0x07*/           6,            '^',
        /*0x08*/           7,            '&',
        /*0x09*/           8,            '*',
        /*0x0a*/           9,            '(',
        /*0x0b*/           0,            ')',
        /*0x0c*/          '-',           '_',
        /*0x0d*/          '=',           '+',
        /*0x0e*/          '\b',           0,
        /*0x0f*/          '\t',           0,

        /*0x10*/          'q',           'Q',
        /*0x11*/          'w',           'W',
        /*0x12*/          'e',           'E',
        /*0x13*/          'r',           'R',
        /*0x14*/          't',           'T',
        /*0x15*/          'y',           'Y',
        /*0x16*/          'u',           'U',
        /*0x17*/          'i',           'I',
        /*0x18*/          'o',           'O',
        /*0x19*/          'p',           'P',
        /*0x1a*/          '[',           '{',
        /*0x1b*/          ']',           '}',
        /*0x1c*/          '\n',          '0',   //Enter
        /*0x1d*/          0x1d,          0x1d,  //Ctrl Left
        /*0x1e*/          'a',           'A',
        /*0x1f*/          's',           'S',
        /*0x20*/          'd',           'D',
        /*0x21*/          'f',           'F',
        /*0x22*/          'g',           'G',
        /*0x23*/          'h',           'H',
        /*0x24*/          'j',           'J',
        /*0x25*/          'k',           'K',
        /*0x26*/          'l',           'L',
        /*0x27*/          ';',           ':',
        /*0x28*/          '\'',          '\"',
        /*0x29*/          '`',           '~',
        /*0x2a*/          0x2a,          0x2a,  //Shift Left
        /*0x2b*/          '\\',          '|',
        /*0x2c*/          'z',           'Z',
        /*0x2d*/          'x',           'X',
        /*0x2e*/          'c',           'C',
        /*0x2f*/          'v',           'V',
        /*0x30*/          'b',           'B',
        /*0x31*/          'n',           'N',
        /*0x32*/          'm',           'M',
        /*0x33*/          ',',           '<',
        /*0x34*/          '.',           '>',
        /*0x35*/          '/',           '?',
        /*0x36*/          0x36,          0x36,  //Shift Right
        /*0x37*/          '*',           '*',
        /*0x38*/          0x38,          0x38,
        /*0x39*/          ' ',           ' ',
        /*0x3a*/           0,             0,    //Caps Lock
        /*0x3b*/           0,             0,    //F1
        /*0x3c*/           0,             0,    //F2
        /*0x3d*/           0,             0,    //F3
        /*0x3e*/           0,             0,    //F4
        /*0x3f*/           0,             0,    //F5
        /*0x40*/           0,             0,    //F6
        /*0x41*/           0,             0,    //F7
        /*0x42*/           0,             0,    //F8
        /*0x43*/           0,             0,    //F9
        /*0x44*/           0,             0,    //F10
        /*0x45*/           0,             0,    //Num Lock
        /*0x46*/           0,             0,    //Scroll Lock
        /*0x47*/          '7',            0,    //Pad Home
        /*0x48*/          '8',            0,    //Pad Up
        /*0x49*/          '9',            0,    //Pad PageUp
        /*0x4a*/          '-',            0,    //Pad Minus
        /*0x4b*/          '4',            0,    //Pad Left
        /*0x4c*/          '5',            0,    //Pad Mid
        /*0x4d*/          '6',            0,    //Pad Right
        /*0x4e*/          '+',            0,    //Pad Plus
        /*0x4f*/          '1',            0,    //Pad End
        /*0x50*/          '2',            0,    //Pad Down
        /*0x51*/          '3',            0,    //Pad PageDown
        /*0x52*/          '0',            0,    //Pad Ins
        /*0x53*/          '.',            0,    //Pad Dot

        /*0x54*/           0,             0,
        /*0x55*/           0,             0,
        /*0x56*/           0,             0,
        /*0x57*/           0,             0,    //F11
        /*0x58*/           0,             0,    //F12
        /*0x59*/           0,             0,
        /*0x5a*/           0,             0,
        /*0x5b*/           0,             0,
        /*0x5c*/           0,             0,
        /*0x5d*/           0,             0,
        /*0x5f*/           0,             0,
        /*0x60*/           0,             0,
        /*0x61*/           0,             0,
        /*0x62*/           0,             0,
        /*0x63*/           0,             0,
        /*0x64*/           0,             0,
        /*0x65*/           0,             0,
        /*0x66*/           0,             0,
        /*0x67*/           0,             0,
        /*0x68*/           0,             0,
        /*0x69*/           0,             0,
        /*0x6a*/           0,             0,
        /*0x6b*/           0,             0,
        /*0x6c*/           0,             0,
        /*0x6d*/           0,             0,
        /*0x6e*/           0,             0,
        /*0x6f*/           0,             0,
        /*0x70*/           0,             0,
        /*0x71*/           0,             0,
        /*0x72*/           0,             0,
        /*0x73*/           0,             0,
        /*0x74*/           0,             0,
        /*0x75*/           0,             0,
        /*0x76*/           0,             0,
        /*0x77*/           0,             0,
        /*0x78*/           0,             0,
        /*0x79*/           0,             0,
        /*0x7a*/           0,             0,
        /*0x7b*/           0,             0,
        /*0x7c*/           0,             0,
        /*0x7d*/           0,             0,
        /*0x7e*/           0,             0,
        /*0x7f*/           0,             0,
};





void keyboard_init(void);
void keyboard_handler(unsigned long nr,unsigned long parameter,struct pt_regs* regs);
void analysis_keycode(void);
unsigned char get_scancode(void);

#endif // !__DEVICE_KEYBOARD_H