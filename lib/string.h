#ifndef __LIB_STRING_H
#define __LIB_STRING_H




static __attribute__((always_inline))
int strlen(char* String) 
{
    /*
    扫描EDI直到与AL的值相等(0)
    */
    register int __res;
    asm volatile (
        "cld    \n\t"
        "repne  \n\t"
        "scasb  \n\t"
        "notl  %0   \n\t"
        "decl  %0   \n\t"
        :"=c"(__res)
        :"D"(String),"a"(0),"0"(0xffffffff)
        :
    );
    return __res;
}



static __attribute__((always_inline))
void* memset(void* ptr,int value,unsigned long size){
    char* p = (char*)ptr;
    while(size--){
        *p++ = value;
    }
    return ptr;
}



char* itoa(char** str,int value,int base);
void* memcpy(void* dest,void* src,unsigned long size);
void* memmove(void* dest,const void* src,unsigned long size);
int strcmp(char* a,char* b);
char* strcpy(char* dst,char* src);
char* strncpy(char* dst,char* src,unsigned int size);
char* strchr(char* str,char c);
char* strcat(char* dst,const char* src);
char* strncat(char* dst,const char* src,unsigned int size);


#endif // !__LIB_STRING_H
