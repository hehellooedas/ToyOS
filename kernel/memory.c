#include <memory.h>
#include <lib.h>
#include <stddef.h>



extern char _text;   //代码段起始地址
extern char _etext;  //代码段结束地址
extern char _edata;  //数据段结束地址
extern char _end;    //内核程序结束地址

unsigned int ZONE_DMA_INDEX = 0;
unsigned int ZONE_NORMAL_INDEX = 0;
unsigned int ZONE_UNMAPED_INDEX = 0;

unsigned long* Global_CR3 = NULL;



void memory_init(void){
    unsigned long TotalMem = 0;
    struct E820* p = NULL;
    color_printk(BLUE, BLACK, "Display Physics Address Map,Type \
    (1:RAM,2:ROM or Reserved,3:ACPI Reclaim Memory,4:ACPI VNS Memory,Others:Undefine)\n");

    p = (struct E820*)(0xffff800000007e00);

    for(int i=0;i<32;i++){
        color_printk(ORANGE, BLACK,\
        "Address:%#018x\tLength:%#018x\tType:%#010x\n",p->address,p->length,p->type);
        if(p->type == 1){
            TotalMem += p->length;
        }
        memory_managerment_struct.e820[i].address += p->address;
        memory_managerment_struct.e820[i].length += p->length;
        memory_managerment_struct.e820[i].type = p->type;
        memory_managerment_struct.e820_length = i;

        p++; //指向下一个ARDS结构(如果下一个结构出现以下条件则直接退出)
        if(p->type > 4 || p->length == 0 || p->type < 1) break;
    }
    color_printk(ORANGE,BLACK,"OS can used Total Ram:%#018x\n",TotalMem);
    TotalMem = 0;
    for(int i=0;i<=memory_managerment_struct.e820_length;i++){
        unsigned long start,end;
        if(memory_managerment_struct.e820[i].type != 1){
            continue;
        }
        start = PAGE_2M_ALIGN(memory_managerment_struct.e820[i].address);
        end = ((memory_managerment_struct.e820[i].address + memory_managerment_struct.e820[i].length) >> PAGE_2M_SHIFT) << PAGE_2M_SHIFT;
        if(end <= start){
            continue;
        } 
        
        TotalMem += (end - start) >> PAGE_2M_SHIFT;
    }
    color_printk(ORANGE,BLACK,"OS can used Total 2M Pages:%#010x=%010d\n",TotalMem,TotalMem);
    
    memory_managerment_struct.start_code = (unsigned long)&_text;
    memory_managerment_struct.end_code = (unsigned long)&_etext;
    memory_managerment_struct.end_data = (unsigned long)&_edata;
    memory_managerment_struct.end_brk = (unsigned long)&_end;
    //color_printk(RED,BLACK,"%x,%x,%x,%x\n",&_text,&_etext,&_edata,&_end);

    

    TotalMem = memory_managerment_struct.e820[memory_managerment_struct.e820_length-2].address + \
    memory_managerment_struct.e820[memory_managerment_struct.e820_length-2].length;
    color_printk(ORANGE,BLACK,"TotalMem = %#018x\n",TotalMem);  //象征性打印一下


    /*  初始化bitmap(跟在内核程序后面)  */
    memory_managerment_struct.bits_map = (unsigned long*)((memory_managerment_struct.end_brk + PAGE_4K_SIZE - 1) & PAGE_4K_MASK);
    memory_managerment_struct.bits_size = TotalMem >> PAGE_2M_SHIFT;
    color_printk(RED,BLACK,"TotalMem = %x,size = %x",TotalMem,TotalMem >> PAGE_2M_SHIFT);
    memory_managerment_struct.bits_length = (((unsigned long)(TotalMem >> PAGE_2M_SHIFT) + sizeof(long) * 8 - 1) / 8) & (~(sizeof(long) - 1));
    memset(memory_managerment_struct.bits_map,0xff,memory_managerment_struct.bits_length);



    /*  初始化Page(跟在bitmap后面)  */
    memory_managerment_struct.pages_struct = (struct Page*)(((unsigned long)memory_managerment_struct.bits_map + memory_managerment_struct.bits_length + PAGE_4K_SIZE - 1) & PAGE_4K_MASK);
    memory_managerment_struct.pages_size = TotalMem >> PAGE_2M_SHIFT;
    memory_managerment_struct.page_length = ((TotalMem >> PAGE_2M_SHIFT) * sizeof(struct Page) + sizeof(long) - 1) & (~(sizeof(long)-1));
    memset(memory_managerment_struct.pages_struct,0x00,memory_managerment_struct.page_length);



    /*  初始化Zone(跟在Page后面)  */
    memory_managerment_struct.zones_struct = (struct Zone*)(((unsigned long)memory_managerment_struct.pages_struct + memory_managerment_struct.page_length + PAGE_4K_SIZE - 1) & PAGE_4K_MASK);
    memory_managerment_struct.zones_size = 0;  //暂时无法确定(所以把Zone放到bitmap和Page的后面)
    memory_managerment_struct.zones_length = (5 * sizeof(struct Zone) + sizeof(long) - 1) & (~(sizeof(long) - 1));
    memset(memory_managerment_struct.zones_struct,0x00,memory_managerment_struct.zones_length);


    for(int i=0;i<=memory_managerment_struct.e820_length;i++){
        unsigned long start,end;
        struct Zone* z;
        struct Page* p;
        unsigned long* b;
        if(memory_managerment_struct.e820[i].type != 1){  //ADRS类型不是1就跳过
            continue;
        }
        start = PAGE_2M_ALIGN(memory_managerment_struct.e820[i].address);
        end = ((memory_managerment_struct.e820[i].address + memory_managerment_struct.e820[i].length) >> PAGE_2M_SHIFT) << PAGE_2M_SHIFT;
        if(end <= start){
            continue;
        }

        //zone init
        z = memory_managerment_struct.zones_struct + memory_managerment_struct.zones_size;
        memory_managerment_struct.zones_size++;  //初值为0
        z->zone_start_address = start;
        z->zone_end_address = end;
        z->zone_length = end - start;

        z->page_using_count = 0;
        z->page_free_count = (end - start) >> PAGE_2M_SHIFT;
        z->total_page_link = 0;

        z->attribute = 0;
        z->GMD_struct = &memory_managerment_struct;

        z->pages_length = (end - start) >> PAGE_2M_SHIFT;
        z->pages_group = (struct Page*)(memory_managerment_struct.pages_struct + (start >> PAGE_2M_SHIFT));


        //page init
        p = z->pages_group;
        for(int j=0;j<z->pages_length;j++,p++){
            p->zone_struct = z;
            p->PHY_address = start + PAGE_2M_SIZE * j;
            p->attribute = 0;

            p->reference_count = 0;
            p->age = 0;
            *(memory_managerment_struct.bits_map + ((p->PHY_address >> PAGE_2M_SHIFT) >> 6)) ^= 1UL << (p->PHY_address >> PAGE_2M_SHIFT) % 64;
        }
    }
    memory_managerment_struct.pages_struct->zone_struct = memory_managerment_struct.zones_struct;
    memory_managerment_struct.pages_struct->PHY_address = 0UL;
    memory_managerment_struct.pages_struct->attribute = 0;
    memory_managerment_struct.pages_struct->reference_count = 0;
    memory_managerment_struct.pages_struct->age = 0;
    memory_managerment_struct.zones_length = (memory_managerment_struct.zones_size * sizeof(struct Zone) + sizeof(long) - 1) & (~(sizeof(long)-1));
    print_memory_manager(memory_managerment_struct);

    ZONE_DMA_INDEX = 0;
    ZONE_NORMAL_INDEX = 0;

    int i;
    for(i=0;i<memory_managerment_struct.zones_size;i++){
        struct Zone* z = memory_managerment_struct.zones_struct + i;
        color_printk(ORANGE,BLACK,\
        "zone_start_address:%#018x,zone_end_address:%#018x,zone_length:%#018x,pages_group:%#018x,pages_length:%#018x\n",\
        z->zone_start_address,z->zone_end_address,z->zone_length,z->pages_group,z->pages_length);
        if(z->zone_start_address == 0x100000000){
            ZONE_DMA_INDEX = i;
        }
    }
    memory_managerment_struct.end_of_struct = (unsigned long)((unsigned long)memory_managerment_struct.zones_struct + memory_managerment_struct.zones_length + sizeof(long) * 32) & (~(sizeof(long) - 1));

    color_printk(ORANGE,BLACK,\
    "end_of_struct:%#018x\n",memory_managerment_struct.end_of_struct);

    i = Virt_To_Phy(memory_managerment_struct.end_of_struct) >> PAGE_2M_SHIFT;
    for(int j = 0;j<=i;j++){
        page_init(memory_managerment_struct.pages_struct + j,PG_PTable_Maped | PG_Kernel_Init | PG_Active | PG_Kernel);
    }
    Global_CR3 = Get_gdt();
    color_printk(INDIGO,BLACK,"Global_CR3\t:%#018x\n",Global_CR3);
    color_printk(INDIGO,BLACK,"*Global_CR3\t:%#018x\n",*Phy_To_Virt(Global_CR3) & (~0xff));
    color_printk(INDIGO,BLACK,"**Global_CR3\t:%#018x\n",*Phy_To_Virt(*Phy_To_Virt(Global_CR3) & (~0xff)) &(~0xff));
    for(i = 0;i<10;i++){
        *(Phy_To_Virt(Global_CR3) + i) = 0UL;
    }
    flush_tlb();
}



unsigned long page_init(struct Page* page,unsigned long flags){
    if(!page->attribute){
        *(memory_managerment_struct.bits_map + ((page->PHY_address >> PAGE_2M_SHIFT) >> 6)) |= 1ul << (page->PHY_address >> PAGE_2M_SHIFT) % 64;
        page->attribute = flags;
        page->reference_count++;
        /*  页所在区域可用页减少/已使用的页增加  */
        page->zone_struct->page_using_count++;
        page->zone_struct->page_free_count--;
        page->zone_struct->total_page_link++;
    }else if((((page->attribute & PG_Referenced) || (page->attribute & PG_K_Share_To_U)) \
    || ((flags & PG_Referenced) || (flags & PG_K_Share_To_U)))){
        page->attribute |= flags;
        page->reference_count++;
        page->zone_struct->total_page_link++;
    }else{
        *(memory_managerment_struct.bits_map + ((page->PHY_address >> PAGE_2M_SHIFT) >> 6)) = 1UL << (page->PHY_address >> PAGE_2M_SHIFT) % 64;
        page->attribute |= flags;
    }
    return 0;
}


struct Page* alloc_pages(int zone_select,int number,unsigned long page_flags){
    int i;
    
}


