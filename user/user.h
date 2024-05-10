#ifndef __USER_USER_H
#define __USER_USER_H




/*  验证输入的地址是否位于用户空间  */
static __attribute__((always_inline))
long verify_user_area(unsigned char* addr,unsigned long size)
{
    if(((unsigned long)addr + size) <= (unsigned long)0x00007fffffffffff){
        return 1;
    }else{
        return 0;
    }
}




/*  从用户空间拷贝数据出来  */
static __attribute__((always_inline))
long copy_from_user(void* from_addr,void* to_addr,unsigned long size)
{
    if(!verify_user_area(from_addr,size)){
        return 0;
    }
    __builtin_memcpy(from_addr,to_addr,size);
    return size;
}



/*  把数据拷贝到用户空间  */
static __attribute__((always_inline))
long copy_to_user(void* from_addr,void* to_addr,unsigned long size)
{
    if(!verify_user_area(to_addr,size)){
        return 0;
    }
    __builtin_memcpy(from_addr,to_addr,size);
    return size;
}



#endif