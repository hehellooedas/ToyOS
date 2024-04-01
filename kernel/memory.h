#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H


#include <list.h>
#include <printk.h>
#include <stdbool.h>

/*
地址范围描述符(ARDS Address Range Descriptior Sturcture)
|字节偏移量 |    属性名称    |    描述    |
0            BaseAddrLow    基地址低32位
4            BaseAddrHigh   基地址高32位
8            LengthLow      内存长度的低32位
12           LengthHigh     内存长度的高32位
16           Type           决定了本段的性质
|----------------------------------------------------|
Type=1意味着该段是可被利用的(其余则不可利用)
Type=2 保留值(ROM映射区)
Type=3 ACPI的回收内存(可以在必要时向固件请求回收这部分内存)
Type=4 ACPINVS内存
*/



/*  符合ARDS数据结构的初步结构体  */
struct Memory_E820_Formate{
    unsigned int addr1;
    unsigned int addr2;
    unsigned int length1;
    unsigned int length2;
    unsigned int type;
};


/*  把两个32位变量表示的地址组合成一个64位的  */
struct E820{
    /*  总共20字节大小  */
    unsigned long address;
    unsigned long length;
    unsigned int type;
}__attribute__((packed));




/*
 * 四级分页机制
 * |  Sign(17)  |   PML4(9)   |   PDPT(9)   |   PDT(9)   |   PT(9)   |   页内偏移(12)   |
 * 63         48 47         39 38         30 29        21 20       12 11               0
 *    0xFFFF8
*/
typedef struct{
    unsigned long pml4t;
} pml4t_t;  //page map level 4
#define mk_pml4t(addr,attr) ((unsigned long)addr | (unsigned long)attr) //为pml4t项设置页属性
#define set_pml4t(pml4tptr,pml4tval) (*(pml4tptr) = (pml4tval))  //将表项设定为指定值


typedef struct{
    unsigned long pdpt;
} pdpt_t;
#define mk_pdpt(addr,attr)      ((unsigned long)addr | (unsigned long)attr)
#define set_pdpt(pdptptr,pdptval)   (*(pdptptr) = (pdptval))


typedef struct{
    unsigned long pdt;
} pdt_t;
#define mk_pdt(addr,attr)      ((unsigned long)addr | (unsigned long)attr)
#define set_pdt(pdtptr,pdtval)  (*(pdtptr) = (pdtval))



/*  用一个统一的结构来描述物理内存分布情况  */
struct Global_Memory_Descriptor{
    struct E820 e820[32];        //每个ARDS描述符的具体情况
    unsigned long e820_length;   //ARDS结构体的数量

    unsigned long*  bits_map;    //物理地址空间页映射位图指针
    unsigned long   bits_size;   //页图的数量
    unsigned long   bits_length; //位图长度

    struct Page* pages_struct;   //指向全局page结构的指针
    unsigned long   pages_size;  //page结构的个数
    unsigned long   page_length; //page结构长度

    struct Zone* zones_struct;   //指向全局zone结构的指针
    unsigned long   zones_size;  //zone结构的个数
    unsigned long   zones_length; //zone结构长度

    unsigned long start_code,end_code,end_data,end_brk; //链接器中设定的重要参数

    unsigned long end_of_struct; //内存页管理结构的结尾地址(重要位置)
};

extern struct Global_Memory_Descriptor memory_management_struct;  //定义在main.c里




#define PTRS_PER_PAGE   512   //页表项个数(每个页表项占8B)
unsigned long* Global_CR3 = NULL;

#define PAGE_OFFSET     ((unsigned long)0xffff800000000000)   //内核层起始线性地址
#define Virt_To_Phy(addr)   ((unsigned long)(addr) - PAGE_OFFSET)  //虚拟地址转换成物理地址
#define Phy_To_Virt(addr)   ((unsigned long*)((unsigned long)(addr) + PAGE_OFFSET))

#define SIZEOF_LONG_ALIGN(size) ((size + sizeof(long) -1) & ~(sizeof(long) - 1))
#define Virt_To_2M_Page(kaddr)  (memory_management_struct.pages_struct + ((Virt_To_Phy(kaddr)) >> PAGE_2M_SHIFT))  //当前虚拟地址属于哪个Page
#define Phy_To_2M_Page(kaddr)   (memory_management_struct.pages_struct + ((unsigned long)kaddr >> PAGE_2M_SHIFT))  //当前物理地址属于哪个Page


#define PAGE_GDT_SHIFT  39
#define PAGE_4K_SHIFT   12
#define PAGE_2M_SHIFT   21
#define PAGE_1G_SHIFT   30

#define PAGE_4K_SIZE    (1UL << PAGE_4K_SHIFT)
#define PAGE_2M_SIZE    (1UL << PAGE_2M_SHIFT)

#define PAGE_4K_MASK    (~(PAGE_4K_SIZE - 1))   //低于4K的部分全部变成0
#define PAGE_2M_MASK    (~(PAGE_2M_SIZE - 1))

#define PAGE_4K_ALIGN(addr) (((unsigned long)(addr) + PAGE_4K_SIZE - 1) & PAGE_4K_MASK)
#define PAGE_2M_ALIGN(addr) (((unsigned long)(addr) + PAGE_2M_SIZE - 1) & PAGE_2M_MASK)





/*  每一个物理页都由该结构来管理  */
struct Page{
    struct Zone* zone_struct;      //本页所属的区域结构体
    unsigned long PHY_address;     //页的物理地址
    unsigned long attribute;       //页的属性
    unsigned long reference_count; //页的引用次数
    unsigned long age;             //页的创建时间
};


#define PAGE_XD     (unsigned long)0x1000000000000000   //禁止执行标志位

#define PAGE_Present (unsigned long) 0x001    //页是否存在(不存在/存在)(在最末端的位置)
#define PAGE_R_W    (unsigned long)0x0002     //页的读写属性(只读/可读可写)
#define PAGE_U_S    (unsigned long)0x0004     //页的访问模式(超级/用户)
#define PAGE_PWT    (unsigned long)0x0008     //页级写穿标志(回写/写穿)
#define PAGE_PCD    (unsigned long)0x0010     //页禁止缓存标志(允许/禁止)
#define PAGE_Accessed (unsigned long)0x0020   //访问标志位(未访问/已访问)
#define PAGE_Dirty  (unsigned long)0x0040     //页脏位(干净/被修改)
#define PAGE_PS     (unsigned long)0x0080     //页尺寸(小页/大页)
#define PAGE_Global (unsigned long)0x0100     //全局页标志位(局部/全局)
#define PAGE_PAT    (unsigned long)0x1000     //页属性(MSR中设置)


#define PAGE_KERNEL_GDT (PAGE_R_W | PAGE_Present)

#define PAGE_KERNEL_Dir  (PAGE_R_W | PAGE_Present)
#define PAGE_KERNEL_Page (PAGE_PS | PAGE_R_W | PAGE_Present)
#define PAGE_USER_GDT    (PAGE_U_S | PAGE_R_W | PAGE_Present)
#define PAGE_USER_Dir    (PAGE_U_S| PAGE_R_W | PAGE_Present)
#define PAGE_USER_Page   (PAGE_PS | PAGE_U_S| PAGE_R_W | PAGE_Present)

#define PG_PTable_Maped	(1 << 0)    //已在页表中映射
#define PG_Kernel_Init	(1 << 1)    //内核初始化程序
#define PG_Device	     (1 << 2)   //设备内存
#define PG_Kernel       (1 << 3)    //内核层空间
#define PG_Shared       (1 << 4)    //已被共享的内存页



/*  一个区域中有多个页,一页可以属于某一个区域  */
struct Zone{
     struct Page*   pages_group;    //数组指针
     unsigned long  pages_length;   //page结构体数量(有几页)

     unsigned long  zone_start_address;  //起始页对齐地址
     unsigned long  zone_end_address;    //结束页对齐地址
     unsigned long  zone_length;         //经过页对齐后的长度
     unsigned long  attribute;           //空间的属性

    struct Global_Memory_Descriptor* GMD_struct;  //指向全局内存描述结构体

    unsigned long   page_using_count;    //已使用物理内存页数量
    unsigned long   page_free_count;     //空闲物理内存页数量

    unsigned long   total_page_link;     //物理页被引用次数
};

#define ZONE_DMA     0x01   //DMA内存区域通常用于支持需要高速数据传输的外部设备
#define ZONE_NORMAL  0x02   //正常的内存区域是大多数应用程序和系统组件所使用的内存(占比最大)
#define ZONE_UNMAPED 0x04   //未映射的内存区域(为系统保留)





/*
Slab是内存管理的基本单位,Slab_cache是管理Slab的数据结构
*/
/*
内存池:管理具体内存对象
管理每个以物理页为单位的内存空间
每个物理页包含了若干个待分配的内存对象
 */
struct Slab{
    struct List list;
    struct Page* page;

    unsigned long using_count;
    unsigned long free_count;

    void* Vaddress;

    /*  颜色位图bitmap  */
    unsigned long color_length;
    unsigned long color_count;

    unsigned long* color_map;
};


/*  对内存池进行整体管理  */
struct Slab_cache{
    unsigned long size;        //当前内存池所管理的内存对象代表的尺寸
    unsigned long total_using;
    unsigned long total_free;  //还有几个size尺寸的内存空间可以分配(所有size尺寸的Slab的free_count之和)

    struct Slab* cache_pool;
    struct Slab* cache_dma_pool;  //用于索引DMA内存池存储空间结构

    void* (*constructor)(void* Vaddress,unsigned long arg);  //内存池构造函数(分配内存块时初始化内存块的内容)
    void* (*destructor)(void* Vaddress,unsigned long arg);   //内存池析构(对象生命周期结束时的清理过程)函数(释放内存块时清理内存块的内容)
};



/*  内存池数组  */
struct Slab_cache kmalloc_cache_size[16] = {
    {32,     0,0,NULL,NULL,NULL,NULL},
    {64,     0,0,NULL,NULL,NULL,NULL},
    {128,    0,0,NULL,NULL,NULL,NULL},
    {256,    0,0,NULL,NULL,NULL,NULL},
    {512,    0,0,NULL,NULL,NULL,NULL},
    {1024,   0,0,NULL,NULL,NULL,NULL},
    {2048,   0,0,NULL,NULL,NULL,NULL},
    {4096,   0,0,NULL,NULL,NULL,NULL},
    {8192,   0,0,NULL,NULL,NULL,NULL},
    {16384,  0,0,NULL,NULL,NULL,NULL},
    {32768,  0,0,NULL,NULL,NULL,NULL},
    {65536,  0,0,NULL,NULL,NULL,NULL},
    {131072, 0,0,NULL,NULL,NULL,NULL},
    {262144, 0,0,NULL,NULL,NULL,NULL},
    {524288, 0,0,NULL,NULL,NULL,NULL},
    {1048576,0,0,NULL,NULL,NULL,NULL}  //1MB
};






/*  输出内存总管理中各结构统计的信息  */
static __attribute__((always_inline))
void print_memory_manager(struct Global_Memory_Descriptor m)
{   
    /*  打印bitmap信息  */
    color_printk(ORANGE,BLACK,\
    "bits_map:%#018x,bits_size:%#018x,bits_length:%#018x\n",\
    m.bits_map,m.bits_size,m.bits_length);

    /*  打印Page信息  */
    color_printk(ORANGE,BLACK,\
    "pages_struct:%#018x,pages_size:%#018x,pages_length:%#018x\n",\
    m.pages_struct,m.pages_size,m.bits_length);

    /*  打印Zone信息  */
    color_printk(ORANGE,BLACK,\
    "zones_struct:%#018x,zones_size:%#018x,zones_length:%#018x\n",\
    m.zones_struct,m.zones_size,m.zones_length);
}



/*  刷新TLB  */
static __attribute__((always_inline))
void flush_tlb(void){
    unsigned long reg;
    asm volatile (
        "movq %%cr3,%0    \n\t"
        "movq %0,%%cr3    \n\t"
        :"=r"(reg)
        :
        :"memory"
    );
}


/*  刷新TLB中某一项  */
static __attribute__((always_inline))
void flush_tlb_one(unsigned long* p)
{
    /*
        Invalidate Page Table Entry 无效化页表项
    */
    asm volatile (
        "invlpg %0  \n\t"
        :
        :"r"(p)
        :"memory"
    );
}




/*  获取控制寄存器cr3的值  */
static  __attribute__((always_inline))
unsigned long* Get_gdt(void)
{
    unsigned long* cr3;
    asm volatile (
        "movq %%cr3,%0  \n\t"
        :"=r"(cr3)
        :
        :"memory"
    );
    return cr3;
}



/*  获取页属性  */
static __attribute__((always_inline))
unsigned long get_page_attribute(struct Page* page)
{
    if(page == NULL){  //页不存在则不能不能查看和设定
        color_printk(RED,BLACK ,"get_page_attribute() ERROR:page == NULL!\n" );
        return 0;
    }else{
        return page->attribute;
    }
}



/*  初始化页  */
static __attribute__((always_inline))
bool page_init(struct Page* page,unsigned long flags)
{
    page->attribute |= flags;
    if(!(page->reference_count ) || (page->attribute & PG_Shared)){
        page->reference_count++;
        page->zone_struct->total_page_link++;
    }
    return true;
}



/*  设置页属性  */
static __attribute__((always_inline))
bool set_page_attribute(struct Page* page,unsigned long flags)
{
    if(page == NULL){
        color_printk(RED,BLACK ,"set_page_attribute() ERROR:page == NULL!\n" );
        return false;
    }else{
        page->attribute = flags;
        return true;
    }
}



static __attribute__((always_inline))
bool page_clean(struct Page* page)
{
    page->reference_count--;
    page->zone_struct->total_page_link--;
    if(!(page->reference_count)){
        page->attribute &= PG_PTable_Maped;
    }
    return true;
}

/*  函数声明  */
void memory_init(void);
void pagetable_init(void);
struct Page* alloc_pages(int zone_select,int number,unsigned long page_flags);
void free_pages(struct Page* page,int number);
bool slab_init(void);
void* slab_malloc(struct Slab_cache* slab_cache,unsigned long arg);
bool slab_free(struct Slab_cache* slab_cache,void* address,unsigned long arg);
void* kmalloc(unsigned long size,unsigned long gfp_flages);
bool kfree(void* address);
struct Slab* kmalloc_create(unsigned long size);


#endif // !__KERNEL_MEMORY_H
