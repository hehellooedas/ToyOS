#include "list.h"
#include <iso646.h>
#include <memory.h>
#include <lib.h>
#include <stddef.h>
#include <printk.h>
#include <stdio.h>



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
    //color_printk(BLUE, BLACK, "Display Physics Address Map,Type \
    (1:RAM,2:ROM or Reserved,3:ACPI Reclaim Memory,4:ACPI VNS Memory,Others:Undefine)\n");

    p = (struct E820*)(0xffff800000007e00); //ARDS缓存地址

    for(int i=0;i<32;i++){  //遍历ARDS寻找所有可用内存
        //color_printk(ORANGE, BLACK,\
        "Address:%#018x\tLength:%#018x\tType:%#010x\n",p->address,p->length,p->type);
        if(p->type == 1){  //type为1才是有效的
            TotalMem += p->length;
        }
        memory_management_struct.e820[i].address += p->address;
        memory_management_struct.e820[i].length += p->length;
        memory_management_struct.e820[i].type = p->type;
        memory_management_struct.e820_length = i;

        p++; //指向下一个ARDS结构(如果下一个结构出现以下条件则直接退出)
        if(p->type > 4 || p->length == 0 || p->type < 1) break;
    }
    color_printk(ORANGE,BLACK,"OS can used Total Ram:%#018x\n",TotalMem);
    TotalMem = 0;
    for(int i=0;i<=memory_management_struct.e820_length;i++){
        unsigned long start,end;
        if(memory_management_struct.e820[i].type != 1){
            continue;
        }
        start = PAGE_2M_ALIGN(memory_management_struct.e820[i].address);  //从当前地址往后的下一个2M页
        /*  该段内存里最后一张可用页的地址(对齐到2MB块)  */
        end = ((memory_management_struct.e820[i].address + memory_management_struct.e820[i].length) >> PAGE_2M_SHIFT) << PAGE_2M_SHIFT;
        if(end <= start){  //如果该段的内存容量不到一页就不使用这段内存了
            continue;
        } 
        
        TotalMem += (end - start) >> PAGE_2M_SHIFT;  //该段内存能提供几个页
    }
    color_printk(ORANGE,BLACK,"OS can used Total 2M Pages:%#010x=%010d\n",TotalMem,TotalMem);
    
    memory_management_struct.start_code = (unsigned long)&_text;
    memory_management_struct.end_code = (unsigned long)&_etext;
    memory_management_struct.end_data = (unsigned long)&_edata;
    memory_management_struct.end_brk = (unsigned long)&_end;
    //color_printk(RED,BLACK,"%x,%x,%x,%x\n",&_text,&_etext,&_edata,&_end);

    

    TotalMem = memory_management_struct.e820[memory_management_struct.e820_length-2].address + \
    memory_management_struct.e820[memory_management_struct.e820_length-2].length;  //总共能使用的内存大小
    color_printk(ORANGE,BLACK,"TotalMem = %#018x\n",TotalMem);  //象征性打印一下


    /*  初始化bitmap(跟在内核程序后面)  */
    memory_management_struct.bits_map = (unsigned long*)((memory_management_struct.end_brk + PAGE_4K_SIZE - 1) & PAGE_4K_MASK);
    memory_management_struct.bits_size = TotalMem >> PAGE_2M_SHIFT;  //总共有几页就需要几个bit
    color_printk(RED,BLACK,"TotalMem = %x,size = %x\n",TotalMem,TotalMem >> PAGE_2M_SHIFT);
    memory_management_struct.bits_length = (((unsigned long)(TotalMem >> PAGE_2M_SHIFT) + sizeof(long) * 8 - 1) / 8) & (~(sizeof(long) - 1));  //需要的位数要用多大的空间来存储
    memset(memory_management_struct.bits_map,0xff,memory_management_struct.bits_length);



    /*  初始化Page(跟在bitmap后面)  */
    memory_management_struct.pages_struct = (struct Page*)(((unsigned long)memory_management_struct.bits_map + memory_management_struct.bits_length + PAGE_4K_SIZE - 1) & PAGE_4K_MASK);
    memory_management_struct.pages_size = TotalMem >> PAGE_2M_SHIFT;
    memory_management_struct.page_length = ((TotalMem >> PAGE_2M_SHIFT) * sizeof(struct Page) + sizeof(long) - 1) & (~(sizeof(long)-1));
    memset(memory_management_struct.pages_struct,0x00,memory_management_struct.page_length);



    /*  初始化Zone(跟在Page后面)  */
    memory_management_struct.zones_struct = (struct Zone*)(((unsigned long)memory_management_struct.pages_struct + memory_management_struct.page_length + PAGE_4K_SIZE - 1) & PAGE_4K_MASK);
    memory_management_struct.zones_size = 0;  //暂时无法确定(所以把Zone放到bitmap和Page的后面)
    memory_management_struct.zones_length = (5 * sizeof(struct Zone) + sizeof(long) - 1) & (~(sizeof(long) - 1));
    memset(memory_management_struct.zones_struct,0x00,memory_management_struct.zones_length);


    for(int i=0;i<=memory_management_struct.e820_length;i++){
        unsigned long start,end;
        struct Zone* z;
        struct Page* p;
        unsigned long* b;

        if(memory_management_struct.e820[i].type != 1){  //ADRS类型不是1就跳过
            continue;
        }
        start = PAGE_2M_ALIGN(memory_management_struct.e820[i].address);
        end = ((memory_management_struct.e820[i].address + memory_management_struct.e820[i].length) >> PAGE_2M_SHIFT) << PAGE_2M_SHIFT;
        if(end <= start){ //不足一页的内存区域跳过去不利用
            continue;
        }  //此时无用的内存区域已经被过滤了

        //zone init
        z = memory_management_struct.zones_struct + memory_management_struct.zones_size; 
        memory_management_struct.zones_size++;  //初值为0
        z->zone_start_address = start;  //当前内存区域的起始地址
        z->zone_end_address = end;      //结束地址
        z->zone_length = end - start;   //长度

        z->page_using_count = 0;
        z->page_free_count = (end - start) >> PAGE_2M_SHIFT;
        z->total_page_link = 0;

        z->attribute = 0;
        z->GMD_struct = &memory_management_struct;

        z->pages_length = (end - start) >> PAGE_2M_SHIFT;
        z->pages_group = (struct Page*)(memory_management_struct.pages_struct + (start >> PAGE_2M_SHIFT));


        //page init
        p = z->pages_group;
        for(int j=0;j<z->pages_length;j++,p++){  //遍历该区域内所有页表
            p->zone_struct = z;
            p->PHY_address = start + PAGE_2M_SIZE * j;  //当前页表的起始地址
            p->attribute = 0;

            p->reference_count = 0;
            p->age = 0;
            /*  将物理页对应的位图里的位标注为未使用状态  */
            *(memory_management_struct.bits_map + ((p->PHY_address >> PAGE_2M_SHIFT) >> 6)) ^= 1UL << (p->PHY_address >> PAGE_2M_SHIFT) % 64;
        }
    }
    memory_management_struct.pages_struct->zone_struct = memory_management_struct.zones_struct;
    memory_management_struct.pages_struct->PHY_address = 0UL;
    memory_management_struct.pages_struct->attribute = 0;
    memory_management_struct.pages_struct->reference_count = 0;
    memory_management_struct.pages_struct->age = 0;
    memory_management_struct.zones_length = (memory_management_struct.zones_size * sizeof(struct Zone) + sizeof(long) - 1) & (~(sizeof(long)-1));
    print_memory_manager(memory_management_struct);

    ZONE_DMA_INDEX = 0;
    ZONE_NORMAL_INDEX = 0;

    int i;
    for(i=0;i<memory_management_struct.zones_size;i++){
        struct Zone* z = memory_management_struct.zones_struct + i;
        color_printk(ORANGE,BLACK,\
        "zone_start_address:%#018x,zone_end_address:%#018x,zone_length:%#018x,pages_group:%#018x,pages_length:%#018x\n",\
        z->zone_start_address,z->zone_end_address,z->zone_length,z->pages_group,z->pages_length);
        if(z->zone_start_address == 0x100000000){
            ZONE_UNMAPED_INDEX = i;  //未映射区域
        }
    }
    memory_management_struct.end_of_struct = (unsigned long)((unsigned long)memory_management_struct.zones_struct + memory_management_struct.zones_length + sizeof(long) * 32) & (~(sizeof(long) - 1));

    color_printk(ORANGE,BLACK,\
    "end_of_struct:%#018x\n",memory_management_struct.end_of_struct);

    i = Virt_To_Phy(memory_management_struct.end_of_struct) >> PAGE_2M_SHIFT;
    for(int j = 0;j<=i;j++){
        /*memory_management_struct实际上在0~2MB内*/
        page_init(memory_management_struct.pages_struct + j,PG_PTable_Maped | PG_Kernel_Init | PG_Active | PG_Kernel);
    }
    Global_CR3 = Get_gdt();
    color_printk(INDIGO,BLACK,"Global_CR3\t:%#018x\n",Global_CR3);
    color_printk(INDIGO,BLACK,"*Global_CR3\t:%#018x\n",*Phy_To_Virt(Global_CR3) & (~0xff));
    color_printk(INDIGO,BLACK,"**Global_CR3\t:%#018x\n",*Phy_To_Virt(*Phy_To_Virt(Global_CR3) & (~0xff)) &(~0xff));
    //for(i = 0;i<10;i++){
        //*(Phy_To_Virt(Global_CR3) + i) = 0UL;
    //}
    flush_tlb();
}



/*  初始化页(标记为已使用状态)  */
unsigned long page_init(struct Page* page,unsigned long flags){
    if(!page->attribute){  //如果页的属性是0(该页未曾使用)
        /* 将当前页对应的物理地址对应的bitmap中的位设置为1 */
        *(memory_management_struct.bits_map + ((page->PHY_address >> PAGE_2M_SHIFT) >> 6)) |= 1UL << (page->PHY_address >> PAGE_2M_SHIFT) % 64;
        page->attribute = flags;
        page->reference_count++;
        /*  页所在区域可用页减少/已使用的页增加  */
        page->zone_struct->page_using_count++;
        page->zone_struct->page_free_count--;
        page->zone_struct->total_page_link++;
    }else if((page->attribute & PG_Referenced) || (page->attribute & PG_K_Share_To_U) || \
    (flags & PG_Referenced) || (flags & PG_K_Share_To_U) ){
        page->attribute |= flags;
        page->reference_count++;
        page->zone_struct->total_page_link++;
    }else{
        *(memory_management_struct.bits_map + ((page->PHY_address >> PAGE_2M_SHIFT) >> 6)) |= 1UL << (page->PHY_address >> PAGE_2M_SHIFT) % 64;
        page->attribute |= flags;
    }
    return 0;
}


/*  页申请函数(一次性最多申请64页)  */
struct Page* alloc_pages(int zone_select,int number,unsigned long page_flags){
    unsigned long page = 0;
    int zone_start = 0;
    int zone_end = 0;
    /*  通过内存区域类型判断去哪些内存区域找  */
    switch (zone_select){
        case ZONE_DMA:  //DMA区域空间(DMA操作可以访问的内存空间)
            zone_start = 0;
            zone_end = ZONE_DMA_INDEX;
            break;

        case ZONE_NORMAL: //已映射页表区域空间
            zone_start = ZONE_DMA_INDEX;
            zone_end = ZONE_NORMAL_INDEX;
            break;

        case ZONE_UNMAPED:  //未映射页表区域空间
            zone_start = ZONE_NORMAL_INDEX;
            zone_end = memory_management_struct.zones_size - 1;
            break;

        default:  //错误的内存区域
            color_printk(RED,BLACK,"alloc_pages error zone_select index\n");
            return NULL;
            break;
    }

    for(int i=zone_start;i<=zone_end;i++){
        struct Zone* z;
        unsigned long start,end,length;
        unsigned long tmp;
        if((memory_management_struct.zones_struct + i)->page_free_count < number){
            continue;  //当前区域的剩余页不足则判断下一个区域
        }
        z = memory_management_struct.zones_struct + i; 

        start = z->zone_start_address >> PAGE_2M_SHIFT; //当前区域管理的起始页
        end = z->zone_end_address >> PAGE_2M_SHIFT;     //当前区域管理的末尾页
        length = z->zone_length >> PAGE_2M_SHIFT;       //当前区域总共管理的页数

        tmp = 64 - start % 64;
        for(int j=start;j<=end;j += j % 64 ? tmp:64){
            unsigned long* p = memory_management_struct.bits_map + (j >> 6);
            unsigned long shift = j % 64;
            for(unsigned long k = shift;k < 64 - shift;k++){
                if(!(((*p >> k) | (*(p+1) << (64-k))) & (number == 64 ? 0xffffffffffffffffUL:((1UL << number)-1)))){
                    page = j + k - 1;
                    for(unsigned long l=0;l<number;l++){
                        struct Page* x = memory_management_struct.pages_struct + page + l;
                        page_init(x,page_flags);
                    }
                    goto find_free_pages;
                }
            }
        }
    }
    return NULL;
find_free_pages:    
    return (struct Page*)(memory_management_struct.pages_struct + page);
}



/*  初始化内存池数组  */
unsigned long slab_init(void)
{
    struct Page* page = NULL;
    unsigned long* virtual = NULL;
    unsigned long i,j;
    unsigned long tmp_address = memory_management_struct.end_of_struct;

    for(i=0;i<16;i++){
        kmalloc_cache_size[i].cache_pool = (struct Slab*)memory_management_struct.end_of_struct;  //slab放置到末尾
        /* 保留一段内存间隙防止出意外  */
        memory_management_struct.end_of_struct += (sizeof(struct Slab) + sizeof(long) * 10);

        list_init(&kmalloc_cache_size[i].cache_pool->list);


        kmalloc_cache_size[i].cache_pool->using_count = 0;
        kmalloc_cache_size[i].cache_pool->free_count = PAGE_2M_SIZE / kmalloc_cache_size[i].size;  //一页能分成几份
        kmalloc_cache_size[i].cache_pool->color_length = ((PAGE_2M_SIZE / kmalloc_cache_size[i].size + sizeof(unsigned long) * 8 - 1) >> 6) << 3;
        kmalloc_cache_size[i].cache_pool->color_count = kmalloc_cache_size[i].cache_pool->free_count;

        kmalloc_cache_size[i].cache_pool->color_map = (unsigned long*)memory_management_struct.end_of_struct;

        memory_management_struct.end_of_struct += (kmalloc_cache_size[i].cache_pool->color_length + sizeof(long) * 10) & (~(sizeof(long) - 1));

        memset(kmalloc_cache_size[i].cache_pool->color_map,0xff,kmalloc_cache_size[i].cache_pool->color_length);

        for(j=0;j<kmalloc_cache_size[i].cache_pool->color_count;j++){
            *(kmalloc_cache_size[i].cache_pool->color_map + (j >> 6)) ^= (1UL << j % 64);
        }

        kmalloc_cache_size[i].total_free = kmalloc_cache_size[i].cache_pool->color_count;
        kmalloc_cache_size[i].total_using = 0;
    }

    i = Virt_To_Phy(memory_management_struct.end_of_struct) >>PAGE_2M_SHIFT;
    for(j=PAGE_2M_ALIGN(Virt_To_Phy(tmp_address));j <= i;j++){
        page = memory_management_struct.pages_struct + j;
        *(memory_management_struct.bits_map + ((page->PHY_address >> PAGE_2M_SHIFT) >> 6)) |= 1UL << (page->PHY_address >>PAGE_2M_SHIFT) %64;
        page->zone_struct->page_using_count++;
        page->zone_struct->page_free_count--;
        page_init(page,PG_PTable_Maped | PG_Kernel_Init | PG_Kernel );
    }

    color_printk(ORANGE,BLACK ,"" );
    for(i=0;i<16;i++){
        virtual = (unsigned long*)((memory_management_struct.end_of_struct + PAGE_2M_SIZE * i + PAGE_2M_SIZE - 1) & PAGE_2M_MASK);
        page = Virt_To_2M_Page(virtual);

        kmalloc_cache_size[i].cache_pool->page = page;
        kmalloc_cache_size[i].cache_pool->Vaddress = virtual;
    }

    return 1;
}




/*  内核层内存分配  */
void* kmalloc(unsigned long size,unsigned long gfp_flages)
{
    int i,j;
    struct Slab* slab = NULL;
    if(size > 1048576){  //1MB(slab内存池数组最大支持1MB)
        color_printk(RED,BLACK ,"kmalloc() ERROR:kmalloc size too long:%d\n",size );
        return NULL;
    }
    for(i=0;i<16;i++){
        if(kmalloc_cache_size[i].size >= size){
            break;
        }
    }
    slab = kmalloc_cache_size[i].cache_pool;
    if(kmalloc_cache_size[i].total_free != 0){
        do{
            if(slab->free_count == 0){  //当前Slab没有空闲了
                slab = container_of(get_List_next(&slab->list),struct Slab ,list ); //去找下一个Slab
            }else{
                break;
            }
        }while(slab != kmalloc_cache_size[i].cache_pool);
    }else{
        //slab = kmalloc_create(kmalloc_cache_size[i].size);
        if(slab == NULL){
            color_printk(BLUE,BLACK ,"kmalloc()->kmalloc_create=>slab == NULL\n" );
            return NULL;
        }
        kmalloc_cache_size[i].total_free += slab->color_count;
    }

    color_printk(RED,BLACK,"kmalloc() ERROR:no memory can alloc\n" );
    color_printk(BLUE,BLACK ,"kmalloc()->kmalloc_create()<=size:%#010x\n",kmalloc_cache_size[i].size );
    list_add_to_before(&kmalloc_cache_size[i].cache_pool->list,&slab->list );

    for(j=0;j<slab->color_count;j++){

    }

    color_printk(BLUE,BLACK ,"kmalloc() ERROR:no memory can alloc\n" );
    return NULL;
}




struct Slab* kmalloc_create(unsigned long size)
{
    return NULL;
}




/*  创建Slab内存池  */
struct Slab_cache* slab_create(unsigned long size,void*(*constructor)(void* Vaddress,unsigned long arg),void*(*destructor)(void* Vaddress,unsigned long arg))
{
    struct Slab_cache* slab_cache = NULL;
    //slab_cache = (struct Slab_cache*)kmalloc(sizeof(struct Slab_cache),0); //从内核空间分配
    if(slab_cache == NULL){ //如果分配失败
        color_printk(RED,BLACK ,"slab_cache内存分配失败!" );
        return NULL;
    }
    memset(slab_cache,0 ,sizeof(struct Slab_cache));

    slab_cache->size = SIZEOF_LONG_ALIGN(size);

    return slab_cache;
}




/*  销毁slab内存池  */
unsigned long slab_destroy(struct Slab_cache* slab_cache)
{
    struct Slab* slab_p = slab_cache->cache_pool;
    struct Slab* tmp_slab = NULL;
    if(slab_cache->total_using != 0){
        color_printk(RED,BLACK ,"slab_cache->total->using != 0!\n" );
        return 0;
    }
    while(!list_is_empty(&slab_p->list)){
        tmp_slab = slab_p;

    }
    return 1;
}



/*  分配Slab内存池中的内存对象  */
void* slab_malloc(struct Slab_cache* slab_cache,unsigned long arg)
{
    return NULL;
}




/*  内存对象归还给内存池  */
unsigned long slab_free(struct Slab_cache* slab_cache)
{
    return 0;
}



