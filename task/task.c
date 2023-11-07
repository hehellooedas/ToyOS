#include <task.h>



void task_init(){
    struct task_struct* p = NULL;
}


unsigned long init(unsigned long arg){
    color_printk(RED,BLACK,\
    "init task is running,arg:%#018x\n",arg);
    return 1;
}



unsigned long do_exit(unsigned long code){
    color_printk(RED,BLACK,\
    "exit task is running,arg:%#018x\n",code);
    while(1);
}