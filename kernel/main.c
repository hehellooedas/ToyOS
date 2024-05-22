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
#include <memory.h>



extern struct ioqueue* keyboard_queue;
extern struct ioqueue* mouse_queue;


void Start_Kernel(void){
    init_all();
    while(1){
        if(keyboard_queue->count){
            analysis_keycode();
        }
        if(mouse_queue->count){
            analysis_mousecode();
        }
    }

    while(1);
}