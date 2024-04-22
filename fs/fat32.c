#include "fat32.h"
#include <memory.h>
#include <lib.h>
#include <disk.h>
#include <string.h>
#include <time.h>
#include <log.h>


struct Disk_Partition_Table DPT;        //硬盘分区表

struct FAT32_BootSector fat32_bootsector;   //FAT32文件系统的启动扇区

struct FAT32_FSInfo fat32_fsinfo;       //辅助扇区


unsigned long FirstDataSector = 0;  //数据区起始扇区
unsigned long BytesPerClus = 0;     //每簇字节数
unsigned long FirstFAT1Sector = 0;  //FAT1表起始扇区
unsigned long FirstFAT2Sector = 0;  //FAT2表起始扇区


void Disk1_FAT32_FS_init()
{
    unsigned char buf[512];
    struct FAT32_Directory* dentry = NULL;

    IDE_device_operation.transfer(ATA_READ,0,1,buf);

    DPT = *(struct Disk_Partition_Table*)buf;

    print_DPTE_info(DPT.DPTE[0]);
    print_DPTE_info(DPT.DPTE[1]);
    print_DPTE_info(DPT.DPTE[2]);
    print_DPTE_info(DPT.DPTE[3]);


    memset(buf,0,512);
    IDE_device_operation.transfer(ATA_READ,DPT.DPTE[0].start_LBA,1,buf);

    fat32_bootsector = *(struct FAT32_BootSector*)buf;

    color_printk(GREEN,BLACK ,"The sector of FSInfo:%d\n",fat32_bootsector.BPB_FSInfo + DPT.DPTE[0].start_LBA );


    /*  获取FSInfo  */
    memset(buf,0,512);
    IDE_device_operation.transfer(ATA_READ,fat32_bootsector.BPB_FSInfo + DPT.DPTE[0].start_LBA,1,buf);
    fat32_fsinfo = *(struct FAT32_FSInfo*)buf;

    /*  判断FSInfo是否正确获取到(记号不对则说明这不是FSInfo)  */
    if(fat32_fsinfo.FSI_LeadSig != 0x41615252 || fat32_fsinfo.FSI_StruSig != 0x61417272){
        log_to_screen(ERROR,"Get Error FSInfo!!!");
    }

    color_printk(GREEN,BLACK ,"freecount:%#x,sig:%#x\n",fat32_fsinfo.FSI_Free_Count );

    FirstDataSector = DPT.DPTE[0].start_LBA + fat32_bootsector.BPB_RsvdSecCnt + fat32_bootsector.BPB_FATSz32 * fat32_bootsector.BPB_NumFATs;

    FirstFAT1Sector = DPT.DPTE[0].start_LBA + fat32_bootsector.BPB_RsvdSecCnt;
    FirstFAT2Sector = FirstFAT1Sector + fat32_bootsector.BPB_FATSz32;
    BytesPerClus = fat32_bootsector.BPB_SecPerClus *fat32_bootsector.BPB_BytesPerSec;

    color_printk(GREEN,BLACK ,"FirstDataSector:%d,FirstFAT1Sector:%d,FirstFAT2Sector:%d",FirstDataSector,FirstFAT1Sector,FirstFAT2Sector );

    dentry = path_walk("/hello.c",0);
    if(dentry != NULL){
        log_to_screen(INFO,"OK ");
        color_printk(GREEN,BLACK,"clusterHI:%#x,cluster_LO:%#x\n",dentry->Dir_FatClusHI,dentry->Dir_FatClusLO);
        //color_printk(GREEN,BLACK ,"find hello.c\nDir_FstClusHI:%#x,nDir_FstClusLO:%#x,Dir_FileSize:%#x\n",dentry->Dir_FatClusHI,dentry->Dir_FatClusLO,dentry->Dir_FileSize );
    }else{
        log_to_screen(WARNING,"Can't find file.");
    }

}



struct FAT32_Directory* path_walk(char* name,unsigned long flags)
{
    char* tmpname = NULL;
    int tmpnamelen = 0;
    struct FAT32_Directory* parent = NULL;
    struct FAT32_Directory* path = NULL;
    char* dentryname = NULL;

    while(*name == '/'){
        name++;
    }
    if(!*name) return NULL;

    parent = (struct FAT32_Directory*)kmalloc(sizeof(struct FAT32_Directory),0 );
    memset(parent,0,sizeof(struct FAT32_Directory));

    dentryname = (char*)kmalloc(PAGE_4K_SIZE,0 );
    memset(dentryname,0,PAGE_4K_SIZE);

    /*  初始的父目录就是第一个目录根目录  */
    parent->Dir_FatClusLO = fat32_bootsector.BPB_RootClus & 0xffff;
    parent->Dir_FatClusHI = (fat32_bootsector.BPB_RootClus >> 16) & 0xfff;

    log_to_screen(INFO,"parent->Dir_FatClusHI:%#x,parent->Dir_FatClusLO:%#x",parent->Dir_FatClusHI,parent->Dir_FatClusLO);

    for(;;){
        tmpname = name;
        while(*name && (*name != '/')) name++;
        tmpnamelen = name - tmpname;        //确定目录名或文件名
        memcpy(dentryname,tmpname ,tmpnamelen );
        dentryname[tmpnamelen] = '\0';

        path = lookup(dentryname,tmpnamelen ,parent ,flags );

        if(path == NULL){   //搜索失败
            log_to_screen(WARNING,"[warning] can't find file or dir:%s",dentryname);
            kfree(dentryname);
            kfree(parent);
            return NULL;
        }
        if(!*name) goto last_component;

        while(*name == '/') name++;

        if(!*name) goto last_slash;

        *parent = *path;
        kfree(path);
    }

    last_component:
    last_slash:
        if(flags & 1){      //返回父目录项
            kfree(dentryname);
            kfree(path);
            return parent;
        }
    kfree(dentryname);
    kfree(parent);
    return path;
}



/*  从指定目录里搜索与目标名匹配的目录项  */
struct FAT32_Directory* lookup(char* name,int namelen,struct FAT32_Directory* dentry,int flags)
{
    unsigned int cluster = 0;
    unsigned long sector = 0;
    unsigned char* buf = kmalloc(BytesPerClus,0 );
    struct FAT32_Directory* p = NULL;
    struct FAT32_Directory* tmpdentry = NULL;
    struct FAT32_LongDirectory* tmpldentry = NULL;
    int j;  //用于存储名称的索引

    cluster = (dentry->Dir_FatClusHI << 16 | dentry->Dir_FatClusLO) & 0xffffffff;
next_cluster:

    /*  通过起始簇号找到对应的扇区  */
    sector = FirstDataSector + (cluster - 2) * fat32_bootsector.BPB_SecPerClus;
    color_printk(GREEN,BLACK ,"lookup cluster:%#x,sector:%#x\n",cluster,sector );

    if(!IDE_device_operation.transfer(ATA_READ,sector,fat32_bootsector.BPB_SecPerClus,buf))
    {
        log_to_screen(ERROR,"[error] FAT32 FS(lookup) read disk ERROR!\n");
        kfree(buf);
        return NULL;
    }

    tmpdentry = (struct FAT32_Directory*)buf;

    for(int i=0;i<BytesPerClus;i+=32,tmpdentry++){    //遍历这个簇的每一个目录项
        if(tmpdentry->Dir_Attr == ATTR_LONG_NAME){
            continue;   //如果是长文件名目录项
        }

        if(tmpdentry->Dir_Name[0] == 0xe5 || tmpdentry->Dir_Name[0] == 0x05 || tmpdentry->Dir_Name[0] == 0x00){ //无效的目录项
            continue;
        }

        /*  筛选出来的就是有效的短目录项  */
        tmpldentry = (struct FAT32_LongDirectory*)tmpdentry - 1;
        j = 0;
        while(tmpldentry->LDIR_Attr == ATTR_LONG_NAME && tmpldentry->LDIR_Ord != 0xe5){
            for(int x=0;x<5;x++){   //匹配文件名的第一部分
                if(j > namelen && tmpldentry->LDIR_Name1[x] == 0xffff){ //0xffff为剩余字符串的填充
                    color_printk(BLUE,BLACK,"%c\n",name[j]);
                    continue;
                }else if(j > namelen || tmpldentry->LDIR_Name1[x] != (unsigned short)(name[j++])){      //出现某个字符不匹配,说明该目录项不是要找的文件的目录项
                    log_to_screen(WARNING,"compare file name fail!");
                    goto continue_cmp_fail;
                }
            }
            for(int x=0;x<6;x++){
                if(j > namelen && tmpldentry->LDIR_Name2[x] == 0xffff){ //0xffff为剩余字符串的填充
                    continue;
                }else if(j > namelen || tmpldentry->LDIR_Name2[x] != (unsigned short)(name[j++])){      //出现某个字符不匹配,说明该目录项不是要找的文件的目录项
                    log_to_screen(WARNING,"compare file name fail!");
                    goto continue_cmp_fail;
                }
            }
            for(int x=0;x<2;x++){
                if(j > namelen && tmpldentry->LDIR_Name3[x] == 0xffff){ //0xffff为剩余字符串的填充
                    continue;
                }else if(j > namelen || tmpldentry->LDIR_Name3[x] != (unsigned short)(name[j++])){      //出现某个字符不匹配,说明该目录项不是要找的文件的目录项
                    log_to_screen(WARNING,"compare file name fail!");
                    goto continue_cmp_fail;
                }
            }
            if(j >= namelen){
                p = (struct FAT32_Directory*)kmalloc(sizeof(struct FAT32_Directory),0 );
                *p = *tmpdentry;
                kfree(buf);
                return p;
            }
            tmpldentry--;
        }

        j = 0;
        color_printk(RED,BLACK,"short dir\n");
        /*  长目录项不一定匹配成功,如果失败了,那么就去匹配短目录项  */
        for(int x=0;x<8;x++){
            switch (tmpdentry->Dir_Name[x]) {
                case ' ':
                    if(!(tmpdentry->Dir_Attr & ATTR_DIRECTORY)){
                        if(name[j] == '.') continue;    //
                        else if(name[j] == tmpdentry->Dir_Name[x]){ //文件名就是带有空格
                            j++;
                            break;
                        }
                        else{
                            log_to_screen(WARNING,"compare file name fail!");
                            goto continue_cmp_fail;    //匹配失败
                        }
                    }else {
                        if(j < namelen && tmpdentry->Dir_Name[x] == name[j]){
                            j++;
                            break;
                        }else if(j == namelen){
                            continue;
                        }else{
                            log_to_screen(WARNING,"compare file name fail!");
                            goto continue_cmp_fail;
                        }
                    }
                case 'A' ... 'Z':
                case 'a' ... 'a':
                    if(tmpdentry->Dir_NTRes & LOWERCASE_BASE){  //与Windows兼容
                        if(j < namelen && tmpdentry->Dir_Name[x] + 32 == name[j]){
                            j++;
                            break;
                        }else{
                            log_to_screen(WARNING,"compare file name fail!");
                            goto continue_cmp_fail;
                        }
                    }else{
                        if(j < namelen && tmpdentry->Dir_Name[x] == name[j]){
                            j++;
                            break;
                        }else{
                            log_to_screen(WARNING,"compare file name fail!");
                            goto continue_cmp_fail;
                        }
                    }
                    break;
                case 0 ... 9:
                    if(j < namelen && tmpdentry->Dir_Name[x] == name[j]){
                        j++;
                        break;
                    }else{
                        log_to_screen(WARNING,"compare file name fail!");
                        goto continue_cmp_fail;
                    }
                default:
                    j++;
                    break;
            }
        }
        /*  匹配扩展名  */
        if(!(tmpdentry->Dir_Attr & ATTR_DIRECTORY)){
            j++;
            for(int x=8;x<11;x++){
                switch (tmpdentry->Dir_Name[x]) {
                    case 'a' ... 'z':
                    case 'A' ... 'Z':
                        if(tmpdentry->Dir_NTRes & LOWERCASE_BASE){
                            if(tmpdentry->Dir_Name[x] + 32 == name[j]){
                                j++;
                                break;
                            }else{
                                log_to_screen(WARNING,"compare file name fail!");
                                goto continue_cmp_fail;
                            }
                        }else{
                            if(tmpdentry->Dir_Name[x] == name[j]){
                                j++;
                                break;
                            }else{
                                log_to_screen(WARNING,"compare file name fail!");
                                goto continue_cmp_fail;
                            }
                        }

                    case '0' ... '9':
                    case ' ':
                        if(tmpdentry->Dir_Name[x] == name[j]){
                            j++;
                            break;
                        }else{
                            log_to_screen(WARNING,"compare file name fail!");
                            goto continue_cmp_fail;
                        }
                    default:
                        log_to_screen(WARNING,"compare file name fail!");
                        goto continue_cmp_fail;
                }
            }

        }

        p = (struct FAT32_Directory*)kmalloc(sizeof(struct FAT32_Directory),0 );
        *p = *tmpdentry;
        kfree(buf);
        return p;


        continue_cmp_fail:;     //匹配失败了
    }

    /*
     *如果搜索完整个簇之后还没有找到对应文件名的目录项
     * 那么就要去父目录的下一个簇号(一个目录并不是只有一个簇号)
     * 在父目录的下一个簇里搜索目标文件名对应的目录项
     * 如此往复,直到搜索到父目录的最后一个簇为止
     */

    cluster = DISK1_FAT32_read_FAT_Entry(cluster);
    if(cluster < 0xffffff7){    //进行下一个簇的匹配(大于这个簇号则不需要再匹配)
        goto next_cluster;
    }
    kfree(buf);
    return NULL;
}



/*  读取FAT指定表项的值  */
unsigned int DISK1_FAT32_read_FAT_Entry(unsigned int fat_entry)
{
    unsigned int buf[128];      //每扇区有128个FAT表项(4B)
    memset(buf,0,512);
    IDE_device_operation.transfer(ATA_READ,FirstFAT1Sector + (fat_entry >> 7),1,(unsigned char*)buf);   //fat_entry >> 7是该目录项所在扇区索引
    log_to_screen(INFO,"fat_entry:%#lx,next fat_entry:%#lx",fat_entry,buf[fat_entry & 0x7f]);
    return buf[fat_entry & 0x7f] & 0xffffffff;  //查询到fat_entry这个表项的值
}



/*  把值写入到指定FAT表项中  */
unsigned long DISK1_FAT32_write_FAT_Entry(unsigned int fat_entry,unsigned int value)
{
    unsigned int buf[128];
    memset(buf,0,512);
    IDE_device_operation.transfer(ATA_READ,FirstFAT1Sector + (fat_entry >> 7),1,(unsigned char*)buf);
    /*  先清空表项原来的值再写入新值  */
    buf[fat_entry & 0x7f] = (buf[fat_entry & 0x7f] & 0xf0000000) | (value & 0xffffffff);

    /*  更新两个FAT表  */
    IDE_device_operation.transfer(ATA_WRITE,FirstFAT1Sector,0,(unsigned char*)buf);
    IDE_device_operation.transfer(ATA_WRITE,FirstFAT2Sector,0,(unsigned char*)buf);
    return 1;
}



