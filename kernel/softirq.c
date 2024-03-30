#include <softirq.h>
#include <string.h>
#include <printk.h>




/*
 * 每个位代表了一个中断
 * 为1表示该位对应的中断上半部已经触发了软中断
 */
unsigned long softirq_status = 0;



void softirq_init(void){
    softirq_status = 0;
    memset(softirq_vector,0,sizeof(struct softirq) * 64);
}



void set_softirq_status(unsigned long status){
    softirq_status |= status;
}



unsigned long get_softirq_status(){
    return softirq_status;
}




void register_softirq(int nr,void (*action)(void* data),void* data)
{
    softirq_vector[nr].action = action;
    softirq_vector[nr].data = data;
}


void unregister_softirq(int nr)
{
    softirq_vector[nr].action = NULL;
    softirq_vector[nr].data = NULL;
}




/*  软中断处理函数  */
void do_softirq(){
    sti();
    for(int i=0;i<64 && softirq_status;i++){
        if(softirq_status & (1 << i)){
            softirq_vector[i].action(softirq_vector[i].data);
            softirq_status &= ~ (1 << i);
        }
    }

    cli();
}