#include <list.h>
#include <iso646.h>
#include <memory.h>
#include <lib.h>
#include <stdbool.h>
#include <stddef.h>
#include <printk.h>
#include <log.h>
#include <spinlock.h>


struct Global_Memory_Descriptor memory_management_struct = {{0},0};


static spinlock_T Page_lock;
static spinlock_T Slab_lock;


extern char _text;   //代码段起始地址
extern char _etext;  //代码段结束地址
extern char _edata;  //数据段结束地址
extern char _end;    //内核程序结束地址


unsigned int ZONE_DMA_INDEX = 0;
unsigned int ZONE_NORMAL_INDEX = 0;
unsigned int ZONE_UNMAPED_INDEX = 0;




void memory_init(void){
    unsigned long TotalMem = 0;
    struct E820* p = NULL;
    //color_printk(BLUE, BLACK, "Display Physics Address Map,Type \
    (1:RAM,2:ROM or Reserved,3:ACPI Reclaim Memory,4:ACPI VNS Memory,Others:Undefine)\n");

    p = (struct E820*)(0xffff800000007e00); //ARDS缓存地址

    for(int i=0;i<32;i++){  //遍历ARDS寻找所有可用内存
        color_printk(ORANGE, BLACK,\
        "Address:%#lx\tLength:%#lx\tType:%#lx\n",p->address,p->length,p->type);
        if(p->type == 1){  //type为1才是有效的
            TotalMem += p->length;
        }
        memory_management_struct.e820[i].address = p->address;
        memory_management_struct.e820[i].length = p->length;
        memory_management_struct.e820[i].type = p->type;
        memory_management_struct.e820_length = i;

        p++; //指向下一个ARDS结构(如果下一个结构出现以下条件则直接退出)
        if(p->type > 4 || p->length == 0 || p->type < 1) break;
    }
    color_printk(ORANGE,BLACK,"OS can used Total Ram:%#lx\n",TotalMem);
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
        
        TotalMem += ((end - start) >> PAGE_2M_SHIFT);  //该段内存能提供几个页
    }
    color_printk(ORANGE,BLACK,"OS can used Total 2M Pages:%#lx=%ld\n",TotalMem,TotalMem);
    
    memory_management_struct.start_code = (unsigned long)&_text;
    memory_management_struct.end_code = (unsigned long)&_etext;
    memory_management_struct.end_data = (unsigned long)&_edata;
    memory_management_struct.end_brk = (unsigned long)&_end;

    

    TotalMem = memory_management_struct.e820[memory_management_struct.e820_length-2].address + \
    memory_management_struct.e820[memory_management_struct.e820_length-2].length;  //总共能使用的内存大小
    color_printk(ORANGE,BLACK,"TotalMem = %#lx\n",TotalMem);  //象征性打印一下

    /*--------------------------------------------------------------------*/

    /*  初始化bitmap(跟在内核程序后面)  */
    memory_management_struct.bits_map = (unsigned long*)((memory_management_struct.end_brk + PAGE_4K_SIZE - 1) & PAGE_4K_MASK);
    memory_management_struct.bits_size = TotalMem >> PAGE_2M_SHIFT;  //总共有几页就需要几个bit
    color_printk(RED,BLACK,"TotalMem = %x,size = %x\n",TotalMem,TotalMem >> PAGE_2M_SHIFT);
    memory_management_struct.bits_length = (((unsigned long)(TotalMem >> PAGE_2M_SHIFT) + sizeof(long) * 8 - 1) / 8) & (~(sizeof(long) - 1));  //需要的位数要用多大的空间来存储
    memset(memory_management_struct.bits_map,0xff,memory_management_struct.bits_length); //暂时设置为分配状态



    /*  初始化Page(跟在bitmap后面)本系统所有页结构都放在这里  */
    memory_management_struct.pages_struct = (struct Page*)(((unsigned long)memory_management_struct.bits_map + memory_management_struct.bits_length + PAGE_4K_SIZE - 1) & PAGE_4K_MASK);
    memory_management_struct.pages_size = TotalMem >> PAGE_2M_SHIFT;
    memory_management_struct.page_length = ((TotalMem >> PAGE_2M_SHIFT) * sizeof(struct Page) + sizeof(long) - 1) & (~(sizeof(long)-1));
    memset(memory_management_struct.pages_struct,0x00,memory_management_struct.page_length);



    /*  初始化Zone(跟在Page后面)  */
    memory_management_struct.zones_struct = (struct Zone*)(((unsigned long)memory_management_struct.pages_struct + memory_management_struct.page_length + PAGE_4K_SIZE - 1) & PAGE_4K_MASK);
    memory_management_struct.zones_size = 0;  //暂时无法确定(所以把Zone放到bitmap和Page的后面)
    memory_management_struct.zones_length = (5 * sizeof(struct Zone) + sizeof(long) - 1) & (~(sizeof(long) - 1));   //留下一部分空间(假设有5个区域)
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
        }
        /*  此时无用的内存区域已经被过滤了  */

        //zone init
        z = memory_management_struct.zones_struct + memory_management_struct.zones_size; 
        memory_management_struct.zones_size++;  //初值为0(初始时未知,根据E820结构体来确定)
        z->zone_start_address = start;  //当前内存区域的起始地址
        z->zone_end_address = end;      //结束地址
        z->zone_length = end - start;   //长度

        z->page_using_count = 0;
        z->page_free_count = (end - start) >> PAGE_2M_SHIFT;    //当前区域可以创建多少个2M页
        z->total_page_link = 0;

        z->attribute = 0;
        z->GMD_struct = &memory_management_struct;

        z->pages_length = (end - start) >> PAGE_2M_SHIFT;
        z->pages_group = (struct Page*)(memory_management_struct.pages_struct + (start >> PAGE_2M_SHIFT));


        //page init 设置该区域的每个页的信息
        p = z->pages_group;
        for(int j=0;j<z->pages_length;j++,p++){  //遍历该区域内所有页表
            p->zone_struct = z;
            p->PHY_address = start + PAGE_2M_SIZE * j;  //当前页表的起始地址
            p->attribute = 0;

            p->reference_count = 0;
            p->age = 0;
            /*  将物理页对应的位图里的位标注为未使用状态(重要)(>> 6 等价于 / 64)  */
            *(memory_management_struct.bits_map + ((p->PHY_address >> PAGE_2M_SHIFT) >> 6)) ^= 1UL << (p->PHY_address >> PAGE_2M_SHIFT) % 64;
        }
    }

    /*  以下不在循环里  */
    memory_management_struct.pages_struct->zone_struct = memory_management_struct.zones_struct;
    memory_management_struct.pages_struct->PHY_address = 0UL;
    set_page_attribute(memory_management_struct.pages_struct,PG_PTable_Maped | PG_Kernel_Init | PG_Kernel );
    memory_management_struct.pages_struct->reference_count = 1;
    memory_management_struct.pages_struct->age = 0;
    memory_management_struct.zones_length = (memory_management_struct.zones_size * sizeof(struct Zone) + sizeof(long) - 1) & (~(sizeof(long)-1));//区域初始化完成后信息需要重新计算,不能再用前面预估的数据了
    print_memory_manager(memory_management_struct);

    ZONE_DMA_INDEX = 0;
    ZONE_NORMAL_INDEX = 0;
    ZONE_UNMAPED_INDEX = 0;

    int i;
    for(i=0;i<memory_management_struct.zones_size;i++){
        struct Zone* z = memory_management_struct.zones_struct + i;
        color_printk(ORANGE,BLACK,\
        "zone_start_address:%#lx,zone_end_address:%#lx,zone_length:%#lx,pages_group:%#lx,pages_length:%#lx\n",\
        z->zone_start_address,z->zone_end_address,z->zone_length,z->pages_group,z->pages_length);
        if(z->zone_start_address == 0x100000000 && !ZONE_UNMAPED_INDEX){
            ZONE_UNMAPED_INDEX = i;  //未映射区域
        }
    }
    memory_management_struct.end_of_struct = (unsigned long)((unsigned long)memory_management_struct.zones_struct + memory_management_struct.zones_length + sizeof(long) * 32) & (~(sizeof(long) - 1));

    color_printk(ORANGE,BLACK,\
    "end_of_struct:%#lx\n",memory_management_struct.end_of_struct);

    i = Virt_To_Phy(memory_management_struct.end_of_struct) >> PAGE_2M_SHIFT;
    for(int j = 0;j<=i;j++){
        /*  memory_management_struct实际上在0~2MB内(第0页)内核增大可能超越(保证内核本身有空间用)  */
        struct Page* tmp_page = memory_management_struct.pages_struct + j;
        page_init(tmp_page,PG_PTable_Maped | PG_Kernel_Init |  PG_Kernel);
        *(memory_management_struct.bits_map + ((tmp_page->PHY_address >> PAGE_2M_SHIFT) >> 6)) |= 1UL << (tmp_page->PHY_address >>PAGE_2M_SHIFT) % 64;
        tmp_page->zone_struct->page_free_count--;
        tmp_page->zone_struct->page_using_count++;
    }

    Global_CR3 = Get_gdt();
    color_printk(INDIGO,BLACK,"Global_CR3\t:%#lx\n",Global_CR3);
    color_printk(INDIGO,BLACK,"*Global_CR3\t:%#lx\n",*Phy_To_Virt(Global_CR3) & (~0xff));
    color_printk(INDIGO,BLACK,"**Global_CR3\t:%#lx\n",*Phy_To_Virt(*Phy_To_Virt(Global_CR3) & (~0xff)) &(~0xff));
    for(i = 0;i<10;i++){
        //*(Phy_To_Virt(Global_CR3) + i) = 0UL;
    }
    flush_tlb();

    spin_init(&Page_lock);
}




/*  页表初始化函数  */
void pagetable_init(void)
{
    unsigned long* tmp;
    Global_CR3 = Get_gdt();
    tmp = (unsigned long*)((unsigned long)(Phy_To_Virt(((unsigned long)Global_CR3) & (~ 0xfffUL))) + 8 * 256);
    color_printk(YELLOW,BLACK ,"1.%#lx,%#lx\n",tmp,*tmp );
    tmp = Phy_To_Virt(*tmp & (~ 0xfffUL));
    color_printk(YELLOW,BLACK ,"2.%#lx,%#lx\n",tmp,*tmp );
    tmp = Phy_To_Virt(*tmp & (~ 0xfffUL));
    color_printk(YELLOW,BLACK ,"3.%#lx,%#lx\n",tmp,*tmp );

    for(unsigned long i=0;i<memory_management_struct.zones_size;i++){
        struct Zone* z = memory_management_struct.zones_struct + i;
        struct Page* p = z->pages_group;
        if(ZONE_UNMAPED_INDEX && i == ZONE_UNMAPED_INDEX){
            break;
        }
        for(unsigned long j=0;j<z->pages_length;j++,p++){
            tmp = (unsigned long*)(((unsigned long)Phy_To_Virt((unsigned long)Global_CR3 & (~ 0xfffUL))) + (((unsigned long)Phy_To_Virt(p->PHY_address) >> PAGE_GDT_SHIFT) & (0x1ff)) * 8);
            if(*tmp == 0){
                unsigned long* virtual = kmalloc(PAGE_4K_SIZE,0);
                set_pml4t(tmp,mk_pml4t(Virt_To_Phy(virtual),PAGE_KERNEL_GDT ) );
            }


            tmp = (unsigned long*)((unsigned long)Phy_To_Virt(*tmp & (~ 0xfffUL))  + (((unsigned long)Phy_To_Virt(p->PHY_address) >> PAGE_1G_SHIFT) & (0x1ff)) * 8);
            if(*tmp == 0){
                unsigned long* virtual = kmalloc(PAGE_4K_SIZE,0);
                set_pdpt(tmp,mk_pdpt(Virt_To_Phy(virtual),PAGE_KERNEL_Dir ) );
            }


            tmp = (unsigned long*)((unsigned long)Phy_To_Virt(*tmp & (~ 0xfffUL))  + (((unsigned long)Phy_To_Virt(p->PHY_address) >> PAGE_2M_SHIFT) & (0x1ff)) * 8);

            set_pdt(tmp,mk_pdt(p->PHY_address,PAGE_KERNEL_Page ) );
        }
    }
    flush_tlb();
}




/*  页申请函数(能申请的页数1~63)  */
struct Page* alloc_pages(int zone_select,int number,unsigned long page_flags){

    if(number >= 64 || number <= 0){
        log_to_screen(ERROR,"alloc_pages() Error:number is invalid");
        return NULL;
    }
    spin_lock(&Page_lock);
    unsigned long page = 0;
    int zone_start = 0;
    int zone_end = 0;

    /*  通过内存区域类型判断去哪些内存区域找  */
    switch (zone_select){
        case ZONE_DMA:    //DMA区域空间(DMA操作可以访问的内存空间)
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
            log_to_screen(ERROR,"alloc_pages error zone_select index");
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
            unsigned long num = (1UL << number) - 1; //需要几页就有几个1

            for(unsigned long k = shift;k < 64;k++){
                if(!((k? ((*p >= k) | (*(p + 1) << (64 - k))):*p) &(num))){
                    page = j + k - shift;
                    for(unsigned long l=0;l<number;l++){
                        struct Page* pageptr = memory_management_struct.pages_struct + page + l;
                        *(memory_management_struct.bits_map + ((pageptr->PHY_address >> PAGE_2M_SHIFT) >> 6)) |= 1UL << (pageptr->PHY_address >>PAGE_2M_SHIFT) % 64;
                        z->page_free_count--;
                        z->page_using_count++;
                        pageptr->attribute = page_flags;
                    }
                    goto find_free_pages;
                }
            }
        }
    }
    log_to_screen(ERROR,"alloc pages fail!");
    spin_unlock(&Page_lock);
    return NULL;
find_free_pages:    
    spin_unlock(&Page_lock);
    return (struct Page*)(memory_management_struct.pages_struct + page);
}




/*  页释放函数  */
void free_pages(struct Page* page,int number)
{
    if(page == NULL){  //页不存在,无需释放
        log_to_screen(ERROR,"free_pages() ERROR:page is invalid");
        return;
    }
    if(number >= 64 || number < 0){ //要释放的页数不合理
        log_to_screen(ERROR,"free_pages() ERROR:number is invalid");
        return;
    }
    spin_lock(&Page_lock);
    for(int i=0;i<number;i++,page++){
        *(memory_management_struct.bits_map + ((page->PHY_address >> PAGE_2M_SHIFT) >> 6)) &= ~(1UL << (page->PHY_address >> PAGE_2M_SHIFT) % 64);
        page->zone_struct->page_free_count++;
        page->zone_struct->page_using_count--;
        page->attribute = 0;
    }
    spin_unlock(&Page_lock);
}



/*  页面检查  */
void page_check()
{

    color_printk(RED,BLACK ,"bitmap_size = %d\n",memory_management_struct.bits_size );
    color_printk(RED,BLACK ,"bitmap_length = %d\n",memory_management_struct.bits_length );
}





/*  初始化内存池数组(kmalloc_cache_size)  */
bool slab_init(void)
{
    struct Page* page = NULL;
    unsigned long* virtual = NULL;
    unsigned long i,j;
    unsigned long tmp_address = memory_management_struct.end_of_struct;  //记录Slab初始化前的末尾

    for(i=0;i<16;i++){
        /*  确定cache_pool的位置  */
        kmalloc_cache_size[i].cache_pool = (struct Slab*)memory_management_struct.end_of_struct;  //slab放置到末尾
        /* 保留一段内存间隙防止出意外(end_of_struct也要跟着后移)  */
        memory_management_struct.end_of_struct += (sizeof(struct Slab) + sizeof(long) * 10);

        list_init(&kmalloc_cache_size[i].cache_pool->list);

        /*  设置cache_pool的基础信息  */
        kmalloc_cache_size[i].cache_pool->using_count = 0;
        kmalloc_cache_size[i].cache_pool->free_count = PAGE_2M_SIZE / kmalloc_cache_size[i].size;  //一页能分成几份

        kmalloc_cache_size[i].cache_pool->color_length = ((PAGE_2M_SIZE / kmalloc_cache_size[i].size + sizeof(unsigned long) * 8 - 1) >> 6) << 3;
        kmalloc_cache_size[i].cache_pool->color_count = kmalloc_cache_size[i].cache_pool->free_count;

        kmalloc_cache_size[i].cache_pool->color_map = (unsigned long*)memory_management_struct.end_of_struct;

        memory_management_struct.end_of_struct += (kmalloc_cache_size[i].cache_pool->color_length + sizeof(long) * 10) & (~(sizeof(long) - 1)); //如法炮制继续后移

        memset(kmalloc_cache_size[i].cache_pool->color_map,0xff,kmalloc_cache_size[i].cache_pool->color_length);

        for(j=0;j<kmalloc_cache_size[i].cache_pool->color_count;j++){
            *(kmalloc_cache_size[i].cache_pool->color_map + (j >> 6)) ^= (1UL << j % 64);
        }

        kmalloc_cache_size[i].total_free = kmalloc_cache_size[i].cache_pool->color_count;
        kmalloc_cache_size[i].total_using = 0;
    }

    /*  如果说后移到了一个新页,那么标记这个页为已使用  */
    i = Virt_To_Phy(memory_management_struct.end_of_struct) >> PAGE_2M_SHIFT;
    for(j=PAGE_2M_ALIGN(Virt_To_Phy(tmp_address)) >> PAGE_2M_SHIFT;j <= i;j++){
        page = memory_management_struct.pages_struct + j;
        *(memory_management_struct.bits_map + ((page->PHY_address >> PAGE_2M_SHIFT) >> 6)) |= 1UL << (page->PHY_address >>PAGE_2M_SHIFT) % 64;
        page->zone_struct->page_using_count++;
        page->zone_struct->page_free_count--;
        page_init(page,PG_PTable_Maped | PG_Kernel_Init | PG_Kernel );
    }

    color_printk(ORANGE,BLACK ,"2.memory_management_struct.bitmap:%x\tzone_struct->page_using_count:%d\tzone_struct->page_free_count:%d\n",*memory_management_struct.bits_map,memory_management_struct.zones_struct->page_using_count,memory_management_struct.zones_struct->page_free_count );

    for(i=0;i<16;i++){
        virtual = (unsigned long*)((memory_management_struct.end_of_struct + PAGE_2M_SIZE * i + PAGE_2M_SIZE - 1) & PAGE_2M_MASK);
        page = Virt_To_2M_Page(virtual);

        *(memory_management_struct.bits_map + ((page->PHY_address >>PAGE_2M_SHIFT) >> 6)) |= 1UL << (page->PHY_address >>PAGE_2M_SHIFT) % 64;

        page->zone_struct->page_using_count++;
        page->zone_struct->page_free_count--;
        page_init(page,PG_PTable_Maped | PG_Kernel_Init | PG_Kernel );

        kmalloc_cache_size[i].cache_pool->page = page;
        kmalloc_cache_size[i].cache_pool->Vaddress = virtual;
    }

    color_printk(ORANGE,BLACK ,"3.memory_management_struct.bitmap:%lx\tzone_struct->page_using_count:%ld\tzone_struct->page_free_count:%ld\n",*memory_management_struct.bits_map,memory_management_struct.zones_struct->page_using_count,memory_management_struct.zones_struct->page_free_count );

    color_printk(ORANGE,BLACK ,"start_code:%lx,end_code:%lx,end_data:%lx,end_brk:%lx,end_of_struct:%lx\n",memory_management_struct.start_code,memory_management_struct.end_code,memory_management_struct.end_data,memory_management_struct.end_brk,memory_management_struct.end_of_struct );

    spin_init(&Slab_lock);
    return true;
}




/*  内核层内存分配(从Slab分配内存)  */
void* kmalloc(unsigned long size,unsigned long gfp_flages)
{
    int i,j;
    struct Slab* slab = NULL;
    if(size > 1048576){  //1MB(slab内存池数组最大支持1MB)
        log_to_screen(ERROR,"kmalloc() ERROR:kmalloc size too long:%d",size);
        return NULL;
    }
    for(i=0;i<16;i++){
        if(kmalloc_cache_size[i].size >= size){ //找到内存池中尺寸刚好大于或等于size的池
            break;
        }
    }
    slab = kmalloc_cache_size[i].cache_pool; //i为选定的内存池的索引
    if(kmalloc_cache_size[i].total_free != 0){
        do{
            if(slab->free_count == 0){  //当前Slab没有空闲了
                slab = container_of(get_List_next(&slab->list),struct Slab ,list ); //去找下一个Slab
            }else{  //slab有空闲就退出
                break;
            }
        }while(slab != kmalloc_cache_size[i].cache_pool);
    }else{
        slab = kmalloc_create(kmalloc_cache_size[i].size);
        if(slab == NULL){
            log_to_screen(WARNING,"kmalloc()->kmalloc_create=>slab == NULL");
            return NULL;
        }
        kmalloc_cache_size[i].total_free += slab->color_count;
        log_to_screen(WARNING,"kmalloc()->kmalloc_create()<=size:%#010x\n",kmalloc_cache_size[i].size);
        list_add_to_before(&kmalloc_cache_size[i].cache_pool->list,&slab->list );
    }

    /*  当前Slab还有空闲  */
    for(j=0;j<slab->color_count;j++){
        if(*(slab->color_map + (j >> 6)) == 0xffffffffffffffffUL){
            j += 63;
            continue;
        }
        if((*(slab->color_map + (j >> 6)) & (1UL << (j % 64))) == 0){
            *(slab->color_map + (j >> 6)) |= 1UL << (j % 64); //标记为使用
            slab->free_count--;
            slab->using_count++;

            kmalloc_cache_size[i].total_free--;
            kmalloc_cache_size[i].total_using++;
            return (void*)((char *)slab->Vaddress + (kmalloc_cache_size[i].size * j));
        }
    }
    log_to_screen(WARNING,"kmalloc() ERROR:no memory can alloc");
    return NULL;
}




/*  分配页内存给Slab  */
struct Slab* kmalloc_create(unsigned long size)
{
    int i;
    struct Slab* slab = NULL;
    struct Page* page = NULL;
    unsigned long* vaddress = NULL;
    long structsize = 0;

    page = alloc_pages(ZONE_NORMAL,1 ,0 );
    if(page == NULL){  //内存分配失败
        log_to_screen(WARNING,"kmalloc_create()->alloc_pages()=>page == NULL");
        return NULL;
    }
    page_init(page,PG_Kernel );

    switch(size){
        /*  小尺寸内存对象  */
        case 32:
        case 64:
        case 128:
        case 256:
        case 512:
            vaddress = Phy_To_Virt(page->PHY_address);
            /*  把Slab内存对象放到页的末尾(小尺寸的情况下color_map占用太多空间)  */
            structsize = sizeof(struct Slab) + PAGE_2M_SIZE / size / 8;
            slab = (struct Slab*)((unsigned char*)vaddress + PAGE_2M_SIZE - structsize);
            slab->color_map = (unsigned long*)((unsigned char*)slab + sizeof(struct Slab));
            slab->free_count = (PAGE_2M_SIZE - (PAGE_2M_SIZE / size / 8) - sizeof(struct Slab)) / size;
            slab->using_count = 0;
            slab->color_count = slab->free_count;
            slab->Vaddress = vaddress;
            slab->page = page;
            list_init(&slab->list);
            slab->color_length = ((slab->color_count + sizeof(unsigned long) * 8 -1) >> 6) << 3;
            memset(slab->color_map,0xff,slab->color_length);
            for(i=0;i<slab->color_count;i++){
                *(slab->color_map + (i << 6)) ^= 1UL << (i % 64);
            }
            break;


        case 1024:
        case 2048:
        case 4096:
        case 8192:
        case 16384:
            /*  中等尺寸对象  */


            /*  大尺寸内存对象  */
        case 32768:
        case 65536:
        case 131072:
        case 262144:
        case 524288:
        case 1048576:
            /*  大尺寸内存对象情况下color_map占用极少的空间,如果还从页里面取就太浪费了  */
            slab = (struct Slab*)kmalloc(sizeof(struct Slab),0 );
            slab->using_count = 0;
            slab->free_count = PAGE_2M_SIZE / size;
            slab->color_count = slab->free_count;
            slab->color_length = ((slab->color_count + (sizeof(unsigned long) * 8) - 1) >> 6) << 3;
            slab->color_map = (unsigned long*)kmalloc(slab->color_length,0 );
            memset(slab->color_map,0xff,slab->color_length);
            slab->Vaddress = Phy_To_Virt(page->PHY_address);
            slab->page = page;
            for(i=0;i<slab->color_count;i++){
                *(slab->color_map + (i >> 6)) ^= 1UL << (i % 64);
            }
            break;


        default:
            log_to_screen(ERROR,"kmalloc_create() Error:worng size:%#lx",size);
            free_pages(page,1);
            return NULL;
    }
    return slab;
}




bool kfree(void* address)
{
    int index; //待释放的内存对象的索引
    struct Slab* slab = NULL;
    void* page_base_address = (void*)((unsigned long)address & PAGE_2M_MASK);
    for(int i=0;i<16;i++){
        slab = kmalloc_cache_size[i].cache_pool;
        do {
            if(slab->Vaddress == page_base_address){
                index = (address - slab->Vaddress) / kmalloc_cache_size[i].size;
                *(slab->color_map + (index >> 6)) ^= 1UL << (index % 64);
                slab->using_count--;
                slab->free_count++;
                kmalloc_cache_size[i].total_using--;
                kmalloc_cache_size[i].total_free++;
            if((slab->using_count == 0) && (kmalloc_cache_size[i].total_free >= slab->color_count * 3 / 2) && (kmalloc_cache_size[i].cache_pool != slab)){
                switch (kmalloc_cache_size[i].size) {
                    case 32:
                    case 64:
                    case 128:
                    case 256:
                    case 512:
                        list_del(&slab->list);
                        kmalloc_cache_size[i].total_free -= slab->free_count;
                        page_clean(slab->page);
                        free_pages(slab->page,1 );
                        break;

                    default:
                        list_del(&slab->list);
                        kmalloc_cache_size[i].total_free -= slab->free_count;
                        kfree(slab->color_map);
                        page_clean(slab->page);
                        free_pages(slab->page,1 );
                        kfree(slab);
                        break;
                }
            }
                return true;
            }else{
                slab = container_of(get_List_next(&slab->list),struct Slab ,list );
            }
        }while(slab != kmalloc_cache_size[i].cache_pool);

    }
    log_to_screen(WARNING,"kfree() Error:can't free memory");
    return false;
}



/*  创建Slab内存池  */
struct Slab_cache* slab_create(
    unsigned long size,\
    void*(*constructor)(void* Vaddress,unsigned long arg),\
    void*(*destructor)(void* Vaddress,unsigned long arg)    \
)
{
    struct Slab_cache* slab_cache = NULL;
    slab_cache = (struct Slab_cache*)kmalloc(sizeof(struct Slab_cache),0); //从内核空间分配
    if(slab_cache == NULL){ //如果分配失败
        log_to_screen(WARNING,"slab_create()->kmalloc()->slab_cache == NULL");
        return NULL;
    }
    memset(slab_cache,0 ,sizeof(struct Slab_cache));

    slab_cache->size = SIZEOF_LONG_ALIGN(size);
    slab_cache->total_free = 0;
    slab_cache->total_free = 0;
    slab_cache->cache_pool = (struct Slab*)kmalloc(sizeof(struct Slab),0);

    if(slab_cache->cache_pool == NULL){
        log_to_screen(WARNING,"slab_create()->kmalloc()->slab_cache == NULL");
        kfree(slab_cache);
        return NULL;
    }
    memset(slab_cache->cache_pool,0,sizeof(struct Slab));

    slab_cache->cache_dma_pool = NULL;
    slab_cache->constructor = constructor;
    slab_cache->destructor = destructor;
    list_init(&slab_cache->cache_pool->list);

    slab_cache->cache_pool->page = (struct Page*)alloc_pages(ZONE_NORMAL,1 ,0 );
    if(slab_cache->cache_pool->page == NULL){
        log_to_screen(WARNING,"slab_create()->alloc_pages()->slab_cache=>cache_pool=>page == NULL\n");
        kfree(slab_cache->cache_pool);
        kfree(slab_cache);
        return NULL;
    }
    page_init(slab_cache->cache_pool->page,PG_Kernel );

    slab_cache->cache_pool->using_count = 0;
    slab_cache->cache_pool->free_count = PAGE_2M_SIZE / slab_cache->size;
    slab_cache->total_free = slab_cache->cache_pool->free_count;
    slab_cache->cache_pool->Vaddress = Phy_To_Virt(slab_cache->cache_pool->page->PHY_address);
    slab_cache->cache_pool->color_count = slab_cache->cache_pool->free_count;
    slab_cache->cache_pool->color_length = ((slab_cache->cache_pool->color_count + sizeof(unsigned long) * 8 - 1) >> 6) << 3; //8字节可以管理64份

    slab_cache->cache_pool->color_map = (unsigned long*)kmalloc(slab_cache->cache_pool->color_length,0);

    if(slab_cache->cache_pool->color_map == NULL){
        log_to_screen(WARNING,"slab_create()->kmalloc()=>slab_cache->cache_pool->color_map == NULL");
        free_pages(slab_cache->cache_pool->page,1 );
        kfree(slab_cache->cache_pool);
        kfree(slab_cache);
        return NULL;
    }

    memset(slab_cache->cache_pool->color_map,0,slab_cache->cache_pool->color_length);
    return slab_cache;
}




/*  销毁slab内存池  */
bool slab_destroy(struct Slab_cache* slab_cache)
{
    struct Slab* slab_p = slab_cache->cache_pool;
    struct Slab* tmp_slab = NULL;
    if(slab_cache->total_using != 0){ //要想释放内存池,则池中的内存对象必须全部空闲
        log_to_screen(WARNING,"slab_cache->total->using != 0!");
        return false;
    }
    while(!list_is_empty(&slab_p->list)){ //递归删除slab链
        tmp_slab = slab_p;
        slab_p = container_of(get_List_next(&slab_p->list),struct Slab ,list );
        list_del(&tmp_slab->list);
        kfree(&tmp_slab->color_map);
        page_clean(tmp_slab->page);
        free_pages(tmp_slab->page,1 );
        kfree(tmp_slab);
    }
    kfree(slab_p->color_map);
    page_clean(slab_p->page);
    free_pages(slab_p->page,1 );
    kfree(slab_p);
    kfree(slab_cache);
    return true;
}



/*  分配Slab内存池中的内存对象(构造)  */
void* slab_malloc(struct Slab_cache* slab_cache,unsigned long arg)
{
    struct Slab* slab_p = slab_cache->cache_pool;
    struct Slab* tmp_slab = NULL;

    if(slab_cache->total_free == 0){ //如果当前内存池中一个内存对象都没了,则该内存池需要扩容
        tmp_slab = (struct Slab*)kmalloc(sizeof(struct Slab),0);
        if(tmp_slab == NULL){
            color_printk(RED,BLACK ,"slab_malloc()->kmalloc()=>tmp_slab == NULL\n" );
            return NULL;
        }
        memset(tmp_slab,0,sizeof(struct Slab));
        list_init(&tmp_slab->list);
        tmp_slab->page = alloc_pages(ZONE_NORMAL,1 ,0 );
        if(tmp_slab->page == NULL){
            log_to_screen(WARNING,"slab_malloc()->alloc_pages()=>tmp_slab->page == NULL");
            kfree(tmp_slab);
            return NULL;
        }
        page_init(tmp_slab->page,PG_Kernel );

        tmp_slab->using_count = 0;
        tmp_slab->free_count = PAGE_2M_SIZE / slab_cache->size;
        tmp_slab->Vaddress = Phy_To_Virt(tmp_slab->page);
        tmp_slab->color_count = tmp_slab->free_count;
        tmp_slab->color_length = ((slab_cache->cache_pool->color_count + sizeof(unsigned long) * 8 - 1) >> 6) << 3;
        tmp_slab->color_map = (unsigned long*)kmalloc(tmp_slab->color_length,0 );
        if(tmp_slab->color_map == NULL){
            log_to_screen(WARNING,"slab_malloc()->kmalloc()=>tmp_cache->color_map == NULL");
            kfree(tmp_slab);
            return NULL;
        }
        memset(tmp_slab->color_map,0,tmp_slab->color_length);
        list_add_to_behind(&slab_cache->cache_pool->list,&tmp_slab->list );
        slab_cache->total_free += tmp_slab->free_count;
        /*  内存池扩容后继续分配内存对象(刚扩容完正常情况下不用往后面找)  */
        for(unsigned int j=0;j<tmp_slab->color_count;j++){
            if((*(tmp_slab->color_map + (j >> 6)) & (1UL << (j % 64))) == 0){
                *(tmp_slab->color_map + (j >> 6)) |= (1UL << (j % 64));
                tmp_slab->using_count++;
                tmp_slab->free_count--;

                slab_cache->total_using++;
                slab_cache->total_free--;
                if(slab_cache->constructor != NULL){
                    return slab_cache->constructor((char*)tmp_slab->Vaddress + slab_cache->size * j,arg);
                }else{
                    return (void*)((char*)tmp_slab->Vaddress + slab_cache->size * j);
                }
            }
        }
    }else{ //无需扩容,遍历内存对象找空闲的
        do{
            if(slab_p->free_count == 0){  //如果没有空闲的内存对象,则找链上的下一个对象
                slab_p = container_of(&slab_p->list,struct Slab ,list );
                continue;
            }
            for(unsigned int j=0;j<tmp_slab->color_count;j++){
                /*  先排除一下没有空闲内存对象的区间(为了加快速度)  */
                if(*(tmp_slab->color_map + (j >> 6)) == 0xffffffffffffffffUL){
                    j += 63;
                    continue;
                }
                if((*(tmp_slab->color_map + (j >> 6)) & (1UL << (j % 64))) == 0){
                    *(tmp_slab->color_map + (j >> 6)) |= (1UL << (j % 64));
                    tmp_slab->using_count++;
                    tmp_slab->free_count--;

                    slab_cache->total_using++;
                    slab_cache->total_free--;
                    if(slab_cache->constructor != NULL){
                        return slab_cache->constructor((char*)tmp_slab->Vaddress + slab_cache->size * j,arg);
                    }else{
                        return (void*)((char*)tmp_slab->Vaddress + slab_cache->size * j);
                    }
                }
            }
        }while(slab_p != slab_cache->cache_pool);
    }

    /*  分配失败  */
    log_to_screen(ERROR,"slab_malloc():Error:can't alloc");
    if(tmp_slab != NULL){
        list_del(&tmp_slab->list);
        kfree(tmp_slab->color_map);
        page_clean(tmp_slab->page);
        free_pages(tmp_slab->page,1 );
        kfree(tmp_slab);
    }
    return NULL;
}




/*  内存对象归还给内存池(析构)  */
bool slab_free(struct Slab_cache* slab_cache,void* address,unsigned long arg)
{
    struct Slab* slab_p = slab_cache->cache_pool;
    int index = 0;
    do{
        /*  待释放的地址在内存池的管理范围之内  */
        if(slab_p->Vaddress < address && address < slab_p->Vaddress + PAGE_2M_SIZE){
            index = (address - slab_p->Vaddress) / slab_cache->size;
            *(slab_p->color_map + (index >> 6)) ^= 1UL << (index % 64); //异或使index位为0
            slab_p->using_count--;
            slab_p->free_count++;

            slab_cache->total_using--;
            slab_cache->total_free++;

            if(slab_cache->destructor != NULL){
                slab_cache->destructor((char*)slab_p->Vaddress + slab_cache->size * index,arg);
            }
            if(slab_p->using_count == 0 && (slab_cache->total_free > slab_p->color_count *3 / 2)){  //内存对象全部空闲或内存对象的量设置的太大,则释放掉内存对象以减少内存占用率
                list_del(&slab_p->list);
                slab_cache->total_free -= slab_p->color_count;
                kfree(slab_p->color_map);
                page_clean(slab_p->page);
                free_pages(slab_p->page,1 );
                kfree(slab_p);
            }
            return true;
        }else{
            slab_p = container_of(get_List_next(&slab_p->list),struct Slab ,list );
            continue;
        }
    }while(slab_p != slab_cache->cache_pool);
    log_to_screen(WARNING,"slab_free() Error:address not in slab");
    return false;
}



