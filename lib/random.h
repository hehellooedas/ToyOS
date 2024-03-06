#ifndef __LIB_RANDOM_H
#define __LIB_RANDOM_H

#include <stdbool.h>


#define RANDOM_RETRY_LOOPS  10  //最多尝试10次

/*
硬件真随机数生成器(HRNG) 生成高质量的真随机数
rdrand返回由加密安全、确定性随机位生成器提供的随机数
rdrand返回时设置CF=1,以指示返回了有效数据
HRNG使用物理随机过程（如电子噪声）来产生熵，然后通过噪声转换电路生成随机数
RDRAND指令可以生成高质量的随机数，并提供的随机性具有很高的熵
在首次使用rdrand指令的时候会初始化相关硬件
*/
static __attribute__((always_inline)) 
bool rdrand_long(unsigned long* value) {
    bool ok;
    unsigned int retry = RANDOM_RETRY_LOOPS;
    do{
        asm volatile (
            "rdrand %[out]"
            :"=@ccc"(ok),[out]"=r"(*value)
            :
            :"memory"
        );
        if(ok) return true;
    }while(retry--);

    return false;
}




/*
硬件种子生成器(HSG) 生成密码学安全的伪随机数
使用较少的CPU指令周期,消耗更少的资源
随机数生成失败的概率比rdrand低
Intel第七代处理器引入该指令(调用该函数前必须进行判断)
*/
static __attribute__((always_inline))
bool rdseed_long(unsigned long* value)
{
    bool ok = false;
    asm volatile (
        "rdseed %[out]"
        :"=@ccc"(ok),[out]"=r"(*value)
        :
        :"memory"
    );
    return ok;
}



/*  通用生成随机数的函数  */
static __attribute__((always_inline))
unsigned long generateRandomNumber(void)
{
    unsigned long random_number;
    if(rdrand_long(&random_number) == 0){
        return 0;
    }
    return random_number;
}


#endif // !__LIB_RANDOM_H