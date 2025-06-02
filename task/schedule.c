#include <SMP.h>
#include <lib.h>
#include <task.h>
#include <schedule.h>
#include <string.h>
#include <printk.h>
#include <interrupt.h>


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
    cli();  //调度期间禁止中断
    struct task_struct* task = NULL;
    struct task_struct* current_task = current;
    __builtin_prefetch(current_task,1,3);
    current_task->flags &= ~NEED_SCHEDULE;   //取消当前进程的可调度标志


    task = get_next_task();
    long cpu_id = SMP_cpu_id();
    __builtin_prefetch(&cpu_id,0,1);
    __builtin_prefetch(&task_schedule[cpu_id],1,3);


    if(current_task->virtual_runtime >= task->virtual_runtime || current_task->state != TASK_RUNNING){
        if(current_task->state == TASK_RUNNING){
            insert_task_queue(current_task);
        }
        if(!task_schedule[cpu_id].CPU_exec_task_jiffies){
            switch (task->priority) {
                case 0:
                case 1:
                    task_schedule[cpu_id].CPU_exec_task_jiffies = 4 / task_schedule[cpu_id].running_task_count;
                    break;
                case 2:     //高优先级给的时间片多
                default:
                    task_schedule[cpu_id].CPU_exec_task_jiffies = 4 / task_schedule[cpu_id].running_task_count * current->priority;
                    break;
            }
        }
        switch_mm(current,task );
        print_pcb_info();
        switch_to(current, task); // 进程切换
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
                    task_schedule[cpu_id].CPU_exec_task_jiffies = 4 / task_schedule[cpu_id].running_task_count * current->priority;
                    break;
            }
        }
    }
    sti();
}



struct task_struct* get_next_task(void)
{
    long cpu_id = SMP_cpu_id();
    struct task_struct* task = NULL;
    if(list_is_empty(&task_schedule[cpu_id].task_queue.list)){
        return init_task[cpu_id];     //队列空了,就默认允许IDLE进程
    }
    task = container_of(get_List_next(&task_schedule[cpu_id].task_queue.list),struct task_struct,list);
    list_del(&task->list);
    task_schedule[cpu_id].running_task_count--;
    return task;
}



void insert_task_queue(struct task_struct* task)
{

    if(task == init_task[SMP_cpu_id()]){ //比较的是地址
        return; //如果一样就不用插入了
    }

    struct task_struct* tmp = NULL;
    tmp = container_of(get_List_next(&task_schedule[SMP_cpu_id()].task_queue.list),struct task_struct,list);

    if(list_is_empty(&task_schedule[SMP_cpu_id()].task_queue.list)){
    }else{
        while (tmp->virtual_runtime < task->virtual_runtime) {
            tmp = container_of(get_List_next(&tmp->list),struct task_struct,list);
        }
    }

    list_add_to_before(&tmp->list,&task->list );
    task_schedule[SMP_cpu_id()].running_task_count++;
}
