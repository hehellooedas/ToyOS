#include <cpu.h>
#include <printk.h>



void cpu_init(void)
{
    unsigned int Max_Basic_Number, Max_Entend_Number;
    unsigned int CpuFacName[4] = { 0, 0, 0, 0 };
    char FactoryName[17] = { 0 };


    /*  查看CPU品牌标志(制造商)  */
    get_cpuid(0, 0, &CpuFacName[0], &CpuFacName[1], &CpuFacName[2],
        &CpuFacName[3]);
    Max_Basic_Number = CpuFacName[0]; // 最大基础功能号(重要)
    *(unsigned int*)&FactoryName[0] = CpuFacName[1];
    *(unsigned int*)&FactoryName[4] = CpuFacName[3];
    *(unsigned int*)&FactoryName[8] = CpuFacName[2];
    *(unsigned int*)&FactoryName[12] = '\0';
    color_printk(YELLOW, BLACK, "%s\t", FactoryName);


    /*  查看CPU具体型号信息  */
    for (int i = 0x80000002; i < 0x80000005; i++) {
        get_cpuid(i, 0, &CpuFacName[0], &CpuFacName[1], &CpuFacName[2],
            &CpuFacName[3]);
        *(unsigned int*)&FactoryName[0] = CpuFacName[0];
        *(unsigned int*)&FactoryName[4] = CpuFacName[1];
        *(unsigned int*)&FactoryName[8] = CpuFacName[2];
        *(unsigned int*)&FactoryName[12] = CpuFacName[3];
        *(unsigned int*)&FactoryName[16] = '\0';
        color_printk(YELLOW, BLACK, "%s", FactoryName);
    }
    color_printk(YELLOW, BLACK, "\n");


    /*  查看CPU类型、家族、型号等信息和特性(重要)  */
    get_cpuid(1, 0, &CpuFacName[0], &CpuFacName[1], &CpuFacName[2],
        &CpuFacName[3]);
    color_printk(YELLOW, BLACK, "Family Code:%#lx,Extended Family:%#lx,\
Model Number:%#lx,Entended Model:%#lx,Processer Type:%#lx,Stepping ID:%#lx\n",
        (CpuFacName[0] >> 8 & 0xf), (CpuFacName[0] >> 20 & 0xff),
        (CpuFacName[0] >> 4 & 0xf), (CpuFacName[0] >> 16 & 0xf),
        (CpuFacName[0] >> 12 & 0x3), (CpuFacName[0] & 0xf));

    /*  ECX和EDX存储处理器支持的机能信息  */
    color_printk(YELLOW,BLACK ,"basic set support:" );
    for(int i=0;i<30;i++){  //打印当前CPU支持的基础指令集
        if(CpuFacName[3] & basic_set.set_Value[i]){
            color_printk(YELLOW,BLACK ,"%s  ",basic_set.set_Name[i] );
        }
    }
    color_printk(YELLOW,BLACK ,"\nextended set support:" );
    for(int i=0;i<31;i++){  //打印当前CPU支持的拓展指令集
        if(CpuFacName[2] & extened_set.set_Value[i]){
            color_printk(YELLOW,BLACK ,"%s  ",extened_set.set_Name[i] );
        }
    }
    color_printk(YELLOW,BLACK ,"\n" );





    /*  CPU可以支持的物理地址和线性地址的位数  */
    get_cpuid(0x80000008, 0, &CpuFacName[0], &CpuFacName[1], &CpuFacName[2],
        &CpuFacName[3]);
    color_printk(YELLOW, BLACK, "Physical Address size:%08d,\
Linear Address size:%08d\n",
        (CpuFacName[0] & 0xff), (CpuFacName[0] >> 8 & 0xff));


    /*  最大基础\拓展功能号  */
    get_cpuid(0x80000000, 0, &CpuFacName[0], &CpuFacName[1], &CpuFacName[2],
        &CpuFacName[3]);
    Max_Entend_Number = CpuFacName[0]; // 最大拓展功能号(重要)
    color_printk(WHITE, BLACK, "Max_Basic_Number = %#x,Max_Entend_Number = %#x\n",
        Max_Basic_Number, Max_Entend_Number);



}
