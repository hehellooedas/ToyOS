#include <printk.h>
#include <string.h>
#include <SMP.h>
#include <cpu.h>
#include <lib.h>
#include <APIC.h>


struct INT_CMD_REG  icr_entry;


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

    color_printk(WHITE,BLACK ,"SMP copy byte:%#lx\n",(unsigned long)&_APU_boot_end - (unsigned long)&_APU_boot_start );

    /*  把AP核心的启动引导写入到指定地址  */
    memcpy((unsigned char*)0xffff800000020000,_APU_boot_start ,(unsigned long)&_APU_boot_end - (unsigned long)&_APU_boot_start );

    wrmsr(0x830,0xc4500 );
    wrmsr(0x830,0xc4620 );
    wrmsr(0x830,0xc4620 );
    stop();
}



void Start_SMP(void){
    unsigned int x,y;
    //color_printk(YELLOW,BLACK ,"APU starting......\n" );

    unsigned long msr = rdmsr(IA32_APIC_BASE_MSR);
    msr |= 0b110000000000;  //开启x2APIC模式
    //msr ^= 0b100000000;
    wrmsr(IA32_APIC_BASE_MSR,msr );


    msr = rdmsr(IA32_APIC_SVR_MSR);
    msr |= 0b0000100000000;  //开启APIC模式(bochs模拟器不支持禁用EOI)
    wrmsr(IA32_APIC_SVR_MSR,msr );

    unsigned long APIC_ID = rdmsr(IA32_APIC_ID_MSR );
    color_printk(GREEN,BLACK ,"APIC ID:%#lx\n",APIC_ID );

    hlt();
}