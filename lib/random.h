#ifndef __LIB_RANDOM_H
#define __LIB_RANDOM_H



/*
生成随机数
rdrand返回由加密安全、确定性随机位生成器提供的随机数
rdrand返回时设置CF=1,以指示返回了有效数据
*/
static __attribute__((always_inline)) 
unsigned long get_random_number() {
    unsigned long rand;
    asm volatile(
        "1: rdrand %0   \n\t"
        "jnc 1b         \n\t"
        : "=r"(rand)
        :
        : "cc"
    );
    return rand;
}



#endif // !__LIB_RANDOM_H