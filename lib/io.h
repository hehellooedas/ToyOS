#ifndef __LIB_IO_H
#define __LIB_IO_H



static __attribute__((always_inline))
unsigned char in8(unsigned short port){
    unsigned char ret;
    asm volatile (
    "inb %%dx,%0  \n\t"
    "mfence       \n\t"
    :"=a"(ret)
    :"d"(port)
    );
    return ret;
}



static __attribute__((always_inline))
unsigned int in32(unsigned short port){
    unsigned int ret;
    asm volatile (
        "inl %%dx,%0    \n\t"
        "mfence         \n\t"
        :"=a"(ret)
        :"d"(port)
    );
    return ret;
}


static __attribute__((always_inline))
void out8(unsigned short port,unsigned char value){
    asm volatile (
        "outb %1,%%dx   \n\t"
        "mfence         \n\t"
        :
        :"d"(port),"a"(value)
        :"memory"
    );
}



static __attribute__((always_inline))
void out32(unsigned short port,unsigned int value){
    asm volatile (
        "outl %1,%%dx   \n\t"
        "mfence         \n\t"
        :
        :"d"(port),"a"(value)
        :"memory"
    );
}



static __attribute__((always_inline))
void insw(unsigned short port,void* buffer,unsigned long nr)
{
    asm volatile (
        "cld        \n\t"
        "rep insw   \n\t"
        "mfence     \n\t"
        :"+D"(buffer),"+c"(nr)
        :"d"(port)
        :"memory"
    );
}



static __attribute__((always_inline))
void outsw(unsigned short port,void* buffer,unsigned long nr)
{
    asm volatile (
        "cld        \n\t"
        "rep outsw  \n\t"
        "mfence     \n\t"
        :"+S"(buffer),"+c"(nr)
        :"d"(port)
        :"memory"
    );
}


#endif // !__LIB_IO_H
