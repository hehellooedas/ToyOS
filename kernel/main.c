#include "screen.h"
#include "task.h"
#include <control.h>
#include <disk.h>
#include <flags.h>
#include <init.h>
#include <interrupt.h>
#include <keyboard.h>
#include <lib.h>
#include <memory.h>
#include <mouse.h>
#include <printk.h>
#include <string.h>


extern struct ioqueue *keyboard_queue;
extern struct ioqueue *mouse_queue;


unsigned long new_process1(unsigned long arg){
  sti();
    color_printk(RED,BLACK,"it is new process1.\n");
    color_printk(RED,BLACK,"new process's pid is %d",get_current()->pid);
    //current->priority = 20;
    
    stop();
    return 10;
}

unsigned long new_process2(unsigned long arg){
  sti();
    color_printk(RED,BLACK,"it is new process2.\n");
    color_printk(RED,BLACK,"new process's pid is %d",get_current()->pid);
    
    stop();
    return 10;
}



void Start_Kernel(void) {
  init_all();
  kernel_thread(new_process1, 1,
                CLONE_FS | CLONE_SIGNAL,20); // 创建init进程(非内核线程)
  kernel_thread(new_process2, 1,
                CLONE_FS | CLONE_SIGNAL,10); // 创建init进程(非内核线程)       
  color_printk(BLUE,BLACK,"become the main.\n");
  while (1) {
    if (keyboard_queue->count) {
      analysis_keycode();
    }
    if (mouse_queue->count) {
      analysis_mousecode();
    }
  }
  while (1)
    ;
}