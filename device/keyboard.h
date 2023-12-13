#ifndef __DEVICE_KEYBOARD_H
#define __DEVICE_KEYBOARD_H


#define KB_BUF_SIZE 100


struct keyboard_inputbuffer{
    unsigned char* p_head;
    unsigned char* o_tail;
    int count;
    unsigned char buf[KB_BUF_SIZE];
};


#define PORT_KB_DATA    0x60
#define PORT_KB_STATUS  0X64



#endif // !__DEVICE_KEYBOARD_H