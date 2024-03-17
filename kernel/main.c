#include <lib.h>
#include <printk.h>
#include <control.h>
#include <init.h>
#include <memory.h>
#include <flags.h>
#include <interrupt.h>

struct Global_Memory_Descriptor memory_management_struct = {{0},0};


void Start_Kernel(void){
    init_all();
    void* tmp = NULL;
    color_printk(YELLOW,BLACK,"Hello\t\t World!\n");
    color_printk(YELLOW,BLACK,"Hello\t\t World!\n");
    color_printk(YELLOW,BLACK,"Hello\t\t World!\n");
    color_printk(YELLOW,BLACK,"Hello\t\t World!\n");

    for(int i=0;i<16;i++){
        color_printk(RED,BLACK ,"size:%x\t",kmalloc_cache_size[i].size );
        color_printk(RED,BLACK ,"color_map(before):%#x\t",*kmalloc_cache_size[i].cache_pool->color_map );
        tmp = kmalloc(kmalloc_cache_size[i].size,0);
        color_printk(RED,BLACK ,"color_map(middle):%#x\t",*kmalloc_cache_size[i].cache_pool->color_map );
        kfree(tmp);
        color_printk(RED,BLACK ,"color_map(after):%#x\n",*kmalloc_cache_size[i].cache_pool->color_map );
    }

    while(1);
}