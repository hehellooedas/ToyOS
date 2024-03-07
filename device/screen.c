#include <screen.h>
#include <memory.h>
#include <string.h>



void frame_buffer_init(void){
    flush_tlb();
}


void screen_init(void){
    int* addr = (int*)0xffff800003000000;
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
    memset(Pos.FB_addr,0 ,Pos.FB_length);
}


/*  滚屏  */
void screen_roll(void){
    char* video_memory_start = (char *)Pos.FB_addr;
    unsigned int step = Pos.XResolution * Pos.YCharSize * 4;
    for(int row=0;row<(Pos.YResolution / Pos.YCharSize);row++){
        memset(video_memory_start+row*step,0,step);
        memcpy(video_memory_start+row*step,video_memory_start+(row+1)*step ,step );
    }
}