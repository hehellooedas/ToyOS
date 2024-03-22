#ifndef __LIB_LIST_H
#define __LIB_LIST_H

#include <printk.h>
#include <stdbool.h>
#include <stddef.h>


/*  
双向链表的基本结构
只需在结构体中定义一个List变量则可加入到队列之中
*/
struct List{
    struct List* prev;
    struct List* next;
};


/*  队列初始化  */
static void __attribute__((always_inline))
list_init(struct List* list){
    list->prev = list;
    list->next = list;
}


/*  插入到后面  */
static void __attribute__((always_inline))
list_add_to_behind(struct List* entry,struct List* newNode)
{
    newNode->next = entry->next;
    newNode->prev = entry;
    entry->next->prev = newNode;
    entry->next = newNode;
}


/*  插入到前面  */
static void __attribute__((always_inline))
list_add_to_before(struct List* entry,struct List* newNode)
{
    newNode->next = entry;
    entry->prev->next = newNode;
    newNode->prev = entry->prev;
    entry->prev = newNode;
}


/*  删除指定链  */
static __attribute__((always_inline))
void list_del(struct List* entry)
{
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
}


/*  判断链表是否为空  */
static __attribute__((always_inline))
bool list_is_empty(struct List* entry)
{
    if(entry->prev == entry && entry->next == entry){
        return true;
    }else{
        return false;
    }
}


/*  获取前驱节点  */
static __attribute__((always_inline))
struct List *get_List_prev(struct List *entry)
{
  if (entry->prev == NULL)
    return NULL;
  else return entry->prev;
}


/*  获取后继节点  */
static __attribute__((always_inline))
struct List *get_List_next(struct List *entry)
{
  if (entry->next == NULL) 
    return NULL;
  else return entry->next;
}


#endif // !__LIB_LIST_H
