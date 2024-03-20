#ifndef __LIB_IOQUEUE_H
#define __LIB_IOQUEUE_H


#include <string.h>

/*
 * 环形队列缓冲区适合 流式数据处理任务
 * 比如鼠标和键盘驱动,键盘控制器发来的数据需要找一个地方缓存
*/

#define buffer_size     200

struct ioqueue{
    unsigned char* head;    //队头指向最后一个放入的数据
    unsigned char* tail;    //队尾指向当前队列里最先放入的数据
    int count;              //有多少个有效字节
    unsigned char buf[buffer_size]; //队列
};




/*  初始化循环队列缓冲区  */
static __attribute__((__always_inline__))
void ioqueue_init(struct ioqueue *ioqueue)
{
    ioqueue->head = ioqueue->tail = ioqueue->buf;
    ioqueue->count = 0;
    memset(ioqueue->buf,buffer_size,0);
}



/*  生产者  */
static __attribute__((always_inline))
void ioqueue_producer(struct ioqueue* ioqueue,unsigned char value)
{
    if(ioqueue->head == ioqueue->buf + buffer_size){
        ioqueue->head = ioqueue->buf;
    }
    *ioqueue->head = value;
    ioqueue->count++;
    ioqueue->head++;
}




/*  消费者  */
static __attribute__((always_inline))
unsigned char ioqueue_consumer(struct ioqueue* ioqueue)
{
    unsigned char ret;
    if(ioqueue->tail == ioqueue->buf + buffer_size){
        ioqueue->tail = ioqueue->buf;
    }
    ret = *ioqueue->tail;
    ioqueue->tail++;
    ioqueue->count--;
    return ret;
}

#endif