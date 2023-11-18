#include <printk.h>
#include <cpu.h>


void cpu_init(void){
    unsigned int Max_Basic_Number,Max_Entend_Number;
    unsigned int CpuFacName[4] = {0,0,0,0};
    char FactoryName[17] = {0};

    get_cpuid(0,0,&CpuFacName[0],&CpuFacName[1],&CpuFacName[2],&CpuFacName[3]);
    Max_Basic_Number = CpuFacName[0];
    *(unsigned int*)&FactoryName[0] = CpuFacName[1];
    *(unsigned int*)&FactoryName[4] = CpuFacName[3];
    *(unsigned int*)&FactoryName[8] = CpuFacName[2];
    *(unsigned int*)&FactoryName[12] = '\0';
    color_printk(YELLOW,BLACK,\
    "%s\t",FactoryName);
    for(int i=0x80000002;i<0x80000005;i++){
        get_cpuid(i,0,&CpuFacName[0],&CpuFacName[1],&CpuFacName[2],&CpuFacName[3]);
        *(unsigned int*)&FactoryName[0] = CpuFacName[0];
        *(unsigned int*)&FactoryName[4] = CpuFacName[1];
        *(unsigned int*)&FactoryName[8] = CpuFacName[2];
        *(unsigned int*)&FactoryName[12] = CpuFacName[3];
        *(unsigned int*)&FactoryName[16] = '\0';
        color_printk(YELLOW,BLACK,"%s",FactoryName);
    }
    color_printk(YELLOW,BLACK,"\n");

    get_cpuid(1,0,&CpuFacName[0],&CpuFacName[1],&CpuFacName[2],&CpuFacName[3]);
    color_printk(YELLOW,BLACK,"Family Code:%#010x,Extended Family:%#010x,\
        Model Number:%#010x,Entended Model:%#010x,Processer Type:%#010x,Stepping ID:%#010x\n",\
        (CpuFacName[0] >> 8 & 0xf),(CpuFacName[0] >> 20 & 0xff),(CpuFacName[0] >>4 & 0xf),\
        (CpuFacName[0] >> 16 & 0xf),(CpuFacName[0] >> 12 & 0x3),(CpuFacName[0] & 0xf));

    get_cpuid(0x80000008,0,&CpuFacName[0],&CpuFacName[1],&CpuFacName[2],&CpuFacName[3]);
    color_printk(YELLOW,BLACK,"Physical Address size:%08d,\
    Linear Address size:%08d\n",(CpuFacName[0] & 0xff),(CpuFacName[0] >> 8 &0xff));

    get_cpuid(0x80000000,0,&CpuFacName[0],&CpuFacName[1],&CpuFacName[2],&CpuFacName[3]);
    Max_Entend_Number = CpuFacName[0];

    color_printk(WHITE,BLACK,"Max_Basic_Number = %#x,Max_Entend_Number = %#x\n",Max_Basic_Number,Max_Entend_Number);
}