#include <memory.h>
#include <timer.h>
#include <softirq.h>
#include <printk.h>



unsigned long jiffies = 0;


void timer_init(void)
{
    struct timer_list* tmp = NULL;
    tmp = (struct timer_list*)kmalloc(sizeof(struct timer_list),0 );
    jiffies = 0;
    init_timer(&timer_list_head,NULL ,NULL ,-1UL );
    register_softirq(0,&do_timer ,NULL );
    init_timer(tmp,&test_timer ,NULL ,10 );
    //add_timer(tmp);
}



void do_timer(void* data)
{
    struct timer_list* tmp = container_of(get_List_next(&timer_list_head.list),struct timer_list,list);
    while((!list_is_empty(&timer_list_head.list)) && (tmp->expire_jiffies <= jiffies)){
        del_timer(tmp);
        tmp->func(tmp->data);
        tmp = container_of(get_List_next(&timer_list_head.list),struct timer_list,list);
    }
    //color_printk(RED,BLACK ,"(HPET:%#lx)\t",jiffies );
}



/*  初始化定时任务队列  */
void init_timer(struct timer_list* timer,void (*func)(void* data),void* data,unsigned long expire_jiffies){
    list_init(&timer->list);
    timer->func = func;
    timer->data = data;
    timer->expire_jiffies = jiffies + expire_jiffies;
}



void add_timer(struct timer_list* timer){
    struct timer_list* tmp = container_of(get_List_next(&timer_list_head.list),struct timer_list,list);
    if(list_is_empty(&timer_list_head.list)){
        //直接插入
    }else{
        while(tmp->expire_jiffies < timer->expire_jiffies){
            tmp = container_of(get_List_next(&tmp->list),struct timer_list,list);
        }
    }
    list_add_to_behind(&tmp->list,&timer->list );
}


/*  删除定时队列  */
void del_timer(struct timer_list* timer){
    list_del(&timer->list);
}



void test_timer(void* data){
    color_printk(GREEN,BLACK ,"timer.c test_timer\n" );
}