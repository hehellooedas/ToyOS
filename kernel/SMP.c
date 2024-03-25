#include <printk.h>
#include <string.h>
#include <SMP.h>
#include <cpu.h>



void SMP_init(void){
    unsigned int a,b,c,d;
    for(int i=0;;i++){
        get_cpuid(0xb,i ,&a ,&b ,&c ,&d );
        if(((c >> 8) & 0xff) == 0){
            break;
        }
        color_printk(WHITE,BLACK,"Local APIC ID Package_../Core_2/SMT_1,type(%#lx)Width:%#lx,num of logical processor(%#lx)\n",c >> 8 & 0xff,a & 0x1f,b & 0xff);
    }
    color_printk(WHITE,BLACK ,"x2APIC ID level:%#lx\tx2APIC ID the current logical processor:%#lx\n",c & 0xff,d );

    color_printk(WHITE,BLACK ,"SMP copy byte:%#lx",(unsigned long)&_APU_boot_end - (unsigned long)&_APU_boot_start );

    /*  把AP核心的启动引导写入到指定地址  */
    memcpy((unsigned char*)0xffff800000020000,_APU_boot_start ,(unsigned long)&_APU_boot_end - (unsigned long)&_APU_boot_start );
}