#include "screen.h"
#include "../kernel/memory.h"



void screen_init(void){
    int* addr = (int*)0xffff800000a00000;
    Pos.XResolution = 1440;
    Pos.YResolution = 920;

    Pos.XPosition = 0;
    Pos.YPosition = 0;

    Pos.XCharSize = 8;
    Pos.YCharSize = 16;

    Pos.FB_addr = (unsigned int*)addr;
    Pos.FB_length = ((Pos.XResolution * Pos.YResolution) * 4 + PAGE_4K_SIZE - 1) & PAGE_4K_MASK;
}