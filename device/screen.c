#include <screen.h>
#include <memory.h>
#include <string.h>
#include <printk.h>
#include <spinlock.h>


void frame_buffer_init(void){
    unsigned long *tmp,*tmp1;
    unsigned int* FB_addr = (unsigned int*)(Phy_To_Virt(0xe0000000));
    Global_CR3 = Get_gdt();

    tmp = Phy_To_Virt((unsigned long*)((unsigned long)Global_CR3 & (~ 0xfffUL)) + (((unsigned long)FB_addr >> PAGE_GDT_SHIFT) & 0x1ff));

    if(*tmp == 0){
        unsigned long* virtual = kmalloc(PAGE_4K_SIZE,0 );
        set_pml4t(tmp,mk_pml4t(Virt_To_Phy(virtual),PAGE_KERNEL_GDT ) );
    }

    tmp = Phy_To_Virt((unsigned long*)(*tmp & (~ 0xfffUL)) + (((unsigned long)FB_addr >> PAGE_1G_SHIFT) & (0x1ff)));
    if(*tmp == 0){
        unsigned long* virtual = kmalloc(PAGE_4K_SIZE,0 );
        set_pdpt(tmp,mk_pdpt(Virt_To_Phy(virtual),PAGE_KERNEL_Dir ) );
    }

    for(unsigned long i=0;i<Pos.FB_length;i+=PAGE_2M_SIZE){
        tmp1 = Phy_To_Virt(((unsigned long*)(*tmp & (~ 0xfffUL))) + (((unsigned long)((unsigned long)FB_addr + i) >>PAGE_2M_SHIFT) & (0x1ff)));
        unsigned long phy = 0xe0000000 + i;
        set_pdt(tmp1,mk_pdt(phy,PAGE_KERNEL_Page | PAGE_PWT | PAGE_PCD ) );
    }

    Pos.FB_addr = (unsigned int*)Phy_To_Virt(0xe0000000);
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

    Pos.XCursorSize = 16;
    Pos.YCursorSize = 16;

    Pos.XCursorPosition = 0;
    Pos.YCursorPosition = 0;

    Pos.FB_addr = (unsigned int*)addr;
    Pos.FB_length = ((Pos.XResolution * Pos.YResolution) * 4 + PAGE_4K_SIZE - 1) & PAGE_4K_MASK; //当前分辨率情况下需要的总字节数

    spin_init(&Pos.printk_lock);
}


/*  清屏  */
void screen_clear(void){
    enum intr_status old_status = spin_lock_irqsave(&Pos.printk_lock);
    /*  设置当前位置为左上角  */
    Pos.XPosition = 0;
    Pos.YPosition = 0;
    /*  把数据清零  */
    memset(Pos.FB_addr,0 ,Pos.FB_length);
    spin_unlock_irqstore(&Pos.printk_lock,old_status);
}




/*  模拟Windows蓝屏(blue screen death)的效果  */
void set_screen_blue(void){
    enum intr_status old_status = spin_lock_irqsave(&Pos.printk_lock);
    Pos.XPosition = 0;
    Pos.YPosition = 0;
    for(int i=0;i<Pos.XResolution * Pos.YResolution;i++){
        color_printk(BLUE,BLUE ," " );
    }
    Pos.XPosition = 0;
    Pos.YPosition = 0;
    spin_unlock_irqstore(&Pos.printk_lock,old_status);
}


/*  滚屏(由printk调用,本身已经上锁)  */
void screen_roll_row(void){
    char* video_memory_start = (char *)Pos.FB_addr;
    unsigned int step = Pos.XResolution * Pos.YCharSize * 4;
    for(unsigned int row=0;row<(Pos.YResolution / Pos.YCharSize);row++){
        //memcpy(video_memory_start+row*step,video_memory_start+(row+1)*step ,step );
        __builtin_memcpy(video_memory_start+row*step,video_memory_start+(row+1)*step,step);
    }
}

