#include "./lib.h"
#include "../lib/printk.h"
#include "gate.h"
#include "init.h"
#include "memory.h"
#include "../lib/flags.h"

struct Global_Memory_Descriptor memory_managerment_struct = {{0},0};


void Start_Kernel(void){
    load_TR(8);
    set_tss64(0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00);
    init_all();
    
    int i;
    color_printk(YELLOW,BLACK,"Hello\t\t World!\n");
    color_printk(YELLOW,BLACK,"Hello\t\t World!\n");
    color_printk(YELLOW,BLACK,"Hello\t\t World!\n");
    color_printk(YELLOW,BLACK,"Hello\t\t World!\n");

    
    while(1);
}