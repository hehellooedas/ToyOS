#include <string.h>
#include <time.h>




char* itoa(char** str,int value,int base)
{
    int m = value % base;
    int q = value / base;
    if(q) itoa(str,q ,base );
    *(*str)++ = (m < 10) ? (m + '0') : (m - 10 + 'A');
    return *str;
}



void* memcpy(void* dest,void* src,unsigned long size)
{
    char* d = (char*)dest;
    char* s = (char*)src;
    while(size--){
        *d++ = *s++;
    }
    return dest;
}



void* memmove(void* dest,const void* src,unsigned long size)
{
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



int strcmp(char* a,char* b)
{
    while(*a != 0 || *a == *b){
        a++;
        b++;
    }
    return *a < *b? -1:*a > *b;
}



char* strcpy(char* dst,char* src)
{
    char* d = dst;
    char* s = src;
    while((*dst++ = *s++));
    return d;
}



char* strncpy(char* dst,char* src,unsigned int size)
{
    char* d = dst;
    char* s = src;
    while((*dst++ = *s++) && size > 0) size--;
    return d;
}




char* strchr(char* str,char c)
{
    if(str == NULL) return NULL;
    while(*str != '\0'){
        if(*str == c){
            return str;
        }
        str++;
    }
    return NULL;
}



char* strcat(char* dst,const char* src)
{
    char* tmp = dst;
    while(*dst) dst++;
    while((*dst++ = *src++) != '\0');
    return tmp;
}


char* strncat(char* dst,const char* src,unsigned int size)
{
    char* tmp = dst;
    if(size){
        while(*dst) dst++;
        while((*dst++ = *src++) != '\0'){
            if(--size == 0){
                *dst = '\0';
                break;
            }
        }
    }
    return tmp;
}

