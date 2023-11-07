#include "string.h"




void* memcpy(void* dest,void* src,unsigned long size){
    char* d = (char*)dest;
    char* s = (char*)src;
    while(size--){
        *d++ = *s++;
    }
    return dest;
}

void* memmove(void* dest,const void* src,unsigned long size){
    char* d = dest;
    const char* s = src;
    if(dest <= src){
        while(size--){
            *d++ = *s++;
        }
    }else{
        d += size;
        s += size;
        while(size--){
            *--d = *--s;
        }
    }

    return dest;
}
