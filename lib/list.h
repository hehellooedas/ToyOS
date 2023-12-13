#ifndef __LIB_LIST_H
#define __LIB_LIST_H

#include <printk.h>
#include <stdbool.h>
#include <stddef.h>

struct List{
    struct List* prev;
    struct List* next;
};


static void __attribute__((always_inline))
list_init(struct List* list){
    list->prev = list;
    list->next = list;
}


static void __attribute__((always_inline))
list_add_to_behind(struct List* entry,struct List* newNode)
{
    newNode->next = entry->next;
    newNode->prev = entry;
    entry->next->prev = newNode;
    entry->next = newNode;
}



static void __attribute__((always_inline))
list_add_to_before(struct List* entry,struct List* newNode)
{
    newNode->next = entry;
    entry->prev->next = newNode;
    newNode->prev = entry->prev;
    entry->prev = newNode;
}



static void __attribute__((always_inline))
list_del(struct List* entry)
{
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
}



static bool __attribute__((always_inline))
list_is_empty(struct List* entry)
{
    if(entry->prev == entry && entry->next == entry){
        return true;
    }else{
        return false;
    }
}


static struct List *__attribute__((always_inline))
get_List_prev(struct List *entry) 
{
  if (entry->prev == NULL)
    return NULL;
  else return entry->prev;
}


static struct List *__attribute__((always_inline))
get_List_next(struct List *entry) 
{
  if (entry->next == NULL) 
    return NULL;
    
  else return entry->next;
}


#endif // !__LIB_LIST_H
