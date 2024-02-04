#include <screen.h>
#include <memory.h>



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


/*  清屏  */
void screen_clear(void){
    /*  设置当前位置为左上角  */
    Pos.XPosition = 0;
    Pos.YPosition = 0;
    /*  把数据清零  */
    unsigned int* addr = Pos.FB_addr;
    for(int i=0;i<Pos.XPosition * Pos.YPosition;i++){
        *addr = 0;
        addr += 1;
    }
}