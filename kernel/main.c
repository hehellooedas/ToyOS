#include <lib.h>
#include <printk.h>
#include <control.h>
#include <init.h>
#include <memory.h>
#include <flags.h>

struct Global_Memory_Descriptor memory_management_struct = {{0},0};


void Start_Kernel(void){
    init_all();
    
    color_printk(YELLOW,BLACK,"Hello\t\t World!\n");
    color_printk(YELLOW,BLACK,"Hello\t\t World!\n");
    color_printk(YELLOW,BLACK,"Hello\t\t World!\n");
    color_printk(YELLOW,BLACK,"Hello\t\t World!\n");
    
    while(1);
}