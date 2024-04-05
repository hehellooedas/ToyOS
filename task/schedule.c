#include "SMP.h"
#include "task.h"
#include <schedule.h>
#include <string.h>
#include <printk.h>


extern unsigned long jiffies;

struct schedule task_schedule[NR_CPUS];


void schedule_init(void)
{
    memset(&task_schedule,0,sizeof(struct schedule) * NR_CPUS);
    for(int i=0;i<NR_CPUS;i++){
        list_init(&task_schedule[i].task_queue.list);
        task_schedule[i].task_queue.virtual_runtime = 0x7fffffffffffffff;
        task_schedule[i].CPU_exec_task_jiffies = 4;
        task_schedule[i].running_task_count = 1;   //默认的IDLE进程
    }

}



void schedule(void)
{
    struct task_struct* task = NULL;
    current->flags &= ~NEED_SCHEDULE;
    task = get_next_task();
    long cpu_id = SMP_cpu_id();
    //color_printk(RED,BLACK ,"#schedule:%#ld#\n",jiffies );

    if(current->virtual_runtime >= task->virtual_runtime){
        if(current->state == TASK_RUNNING){
            insert_task_queue(current);
        }
        if(!task_schedule[cpu_id].CPU_exec_task_jiffies){
            switch (task->priority) {
                case 0:
                case 1:
                    task_schedule[cpu_id].CPU_exec_task_jiffies = 4 / task_schedule[cpu_id].running_task_count;
                    break;
                case 2:     //高优先级给的时间片多
                default:
                    task_schedule[cpu_id].CPU_exec_task_jiffies = 4 / task_schedule[cpu_id].running_task_count * 3;
                    break;
            }
        }
        switch_to(current,task );
    }else{
        insert_task_queue(task);
        if(!task_schedule[cpu_id].CPU_exec_task_jiffies){
            switch (task->priority) {
                case 0:
                case 1:
                    task_schedule[cpu_id].CPU_exec_task_jiffies = 4 / task_schedule[cpu_id].running_task_count;
                    break;
                case 2:
                default:
                    task_schedule[cpu_id].CPU_exec_task_jiffies = 4 / task_schedule[cpu_id].running_task_count * 3;
                    break;
            }
        }
    }
}



struct task_struct* get_next_task(void)
{
    struct task_struct* task = NULL;
    if(list_is_empty(&task_schedule[SMP_cpu_id()].task_queue.list)){
        return &init_task_union.task;
    }
    task = container_of(get_List_next(&task_schedule[SMP_cpu_id()].task_queue.list),struct task_struct,list);
    list_del(&task->list);
    task_schedule[SMP_cpu_id()].running_task_count--;
    return task;
}



void insert_task_queue(struct task_struct* task)
{
    struct task_struct* tmp = container_of(get_List_next(&task_schedule[SMP_cpu_id()].task_queue.list),struct task_struct,list);
    if(task == tmp){
        return;
    }
    if(list_is_empty(&task_schedule[SMP_cpu_id()].task_queue.list)){

    }else{
        while (tmp->virtual_runtime < task->virtual_runtime) {
            tmp = container_of(get_List_next(&tmp->list),struct task_struct,list);
        }
    }
    list_add_to_before(&tmp->list,&task->list );
    task_schedule[SMP_cpu_id()].running_task_count++;
}