#include <lib.h>
#include <printk.h>
#include <control.h>
#include <init.h>
#include <memory.h>
#include <flags.h>
#include <interrupt.h>
#include <keyboard.h>
#include <mouse.h>
#include <disk.h>

struct Global_Memory_Descriptor memory_management_struct = {{0},0};

extern struct ioqueue* keyboard_queue;
extern struct ioqueue* mouse_queue;
extern struct block_device_operation IDE_device_operation;

void Start_Kernel(void){
    init_all();
    *(unsigned int*)0xffff800000020000 = 0xf4;
    wrmsr(0x830,0xc4500 );
    wrmsr(0x830,0xc4620 );
    wrmsr(0x830,0xc4620 );
    /*
    while(1){
        if(keyboard_queue->count){
            analysis_keycode();
        }
        if(mouse_queue->count){
            analysis_mousecode();
        }
    }
    */
    while(1);
}