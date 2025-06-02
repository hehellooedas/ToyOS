#include "fat32.h"
#include "../posix/errno.h"
#include "../posix/stdio.h"
#include "VFS.h"
#include "printk.h"
#include <disk.h>
#include <lib.h>
#include <list.h>
#include <memory.h>
#include <string.h>
#include <time.h>

struct dir_entry *FAT32_lookup(struct index_node *parent_inode,
                               struct dir_entry *destination_dentry);
long FAT32_create(struct index_node *inode, struct dir_entry *dentry);
long FAT32_mkdir(struct index_node *inode, struct dir_entry *dentry, int mode);
long FAT32_rmdir(struct index_node *inode, struct dir_entry *dentry);
long FAT32_rename(struct index_node *old_inode, struct dir_entry *old_dentry,
                  struct index_node *new_inode, struct dir_entry *new_dentry);
long FAT32_getattr(struct dir_entry *dentry, unsigned long *attr);
long FAT32_setattr(struct dir_entry *dentry, unsigned long *attr);

struct Disk_Partition_Table DPT; // 硬盘分区表

struct FAT32_BootSector fat32_bootsector; // FAT32文件系统的启动扇区

struct FAT32_FSInfo fat32_fsinfo; // 辅助扇区

unsigned long FirstDataSector = 0; // 数据区起始扇区
unsigned long BytesPerClus = 0;    // 每簇字节数
unsigned long FirstFAT1Sector = 0; // FAT1表起始扇区
unsigned long FirstFAT2Sector = 0; // FAT2表起始扇区

struct file_system_type FAT32_fs_type = {.name = "FAT32",
                                         .fs_flags = 0,
                                         .read_superblock =
                                             fat32_read_superblock,
                                         .next = NULL};

void Disk1_FAT32_FS_init() {
  unsigned char buf[512];
  struct dir_entry *dentry = NULL;
  struct Disk_Partition_Table DPT = {0};

  register_filesystem(&FAT32_fs_type);

  /*  读取引导扇区  */
  memset(buf, 0, 512);
  IDE_device_operation.transfer(ATA_READ, 0, 1, buf);

  DPT = *(struct Disk_Partition_Table *)buf;

  memset(buf, 0, 512);
  IDE_device_operation.transfer(ATA_READ, DPT.DPTE[0].start_LBA, 1, buf);

  root_sb = mount_fs("FAT32", &DPT.DPTE[0], buf); // 在挂载的时候会解析MBR的内容

  dentry = path_walk("/test1_dir/test2_dir/test3_dir/cpu.c", 0);
  if (dentry != NULL) {
    log_to_screen(INFO, "Find the file!");
    nop();
  } else {
    log_to_screen(WARNING, "Can't find file.");
  }
  color_printk(WHITE, BLACK, "[info] cpu.c's parent directory is :%s\n",
                dentry->parent->name);
  color_printk(WHITE, BLACK, "[info] cpu.c's parent's parent directory is :%s\n",
                dentry->parent->parent->name);
  color_printk(WHITE, BLACK, "[info] cpu.c's parent's parent's parent directory is :%s\n",
                dentry->parent->parent->parent->name);
  color_printk(WHITE, BLACK, "[info] cpu.c's parent's parent's parent's parent directory is :%s\n",
                dentry->parent->parent->parent->parent->name);
}

/*  读取FAT指定表项的值  */
unsigned int DISK1_FAT32_read_FAT_Entry(struct FAT32_sb_info *fsbi,
                                        unsigned int fat_entry) {
  unsigned int buf[128]; // 每扇区有128个FAT表项(4B)
  memset(buf, 0, 512);
  IDE_device_operation.transfer(
      ATA_READ, fsbi->FAT1_firstsector + (fat_entry >> 7), 1,
      (unsigned char *)buf); // fat_entry >> 7是该目录项所在扇区索引
  log_to_screen(INFO, "fat_entry:%#lx,next fat_entry:%#lx", fat_entry,
                buf[fat_entry & 0x7f]);
  return buf[fat_entry & 0x7f] & 0xffffffff; // 查询到fat_entry这个表项的值
}

/*  把值写入到指定FAT表项中  */
unsigned long DISK1_FAT32_write_FAT_Entry(struct FAT32_sb_info *fsbi,
                                          unsigned int fat_entry,
                                          unsigned int value) {
  unsigned int buf[128];
  memset(buf, 0, 512);
  IDE_device_operation.transfer(ATA_READ,
                                fsbi->FAT1_firstsector + (fat_entry >> 7), 1,
                                (unsigned char *)buf);
  /*  先清空表项原来的值再写入新值  */
  buf[fat_entry & 0x7f] =
      (buf[fat_entry & 0x7f] & 0xf0000000) | (value & 0xffffffff);

  /*  更新两个FAT表  */
  IDE_device_operation.transfer(ATA_WRITE, fsbi->FAT1_firstsector, 0,
                                (unsigned char *)buf);
  IDE_device_operation.transfer(ATA_WRITE, fsbi->FAT1_firstsector, 0,
                                (unsigned char *)buf);

  return 1;
}

void fat32_put_superblock(struct super_block *sb) {
  kfree(sb->private_sb_info);
  kfree(sb->root->dir_inode->private_index_info);
  kfree(sb->root->dir_inode);
  kfree(sb->root);
  kfree(sb);
}

/*  将修改后的inode结构写回到硬盘扇区中(inode携带新信息)  */
void fat32_write_inode(struct index_node *inode) {
  struct FAT32_Directory *fdentry = NULL;
  struct FAT32_Directory *buf = NULL;
  struct FAT32_inode_info *finode = inode->private_index_info;
  struct FAT32_sb_info *fsbi = inode->sb->private_sb_info;
  unsigned long sector = 0;

  if (finode->dentry_location == 0) {
    log_to_screen(WARNING, "FS ERROR:write root inode!");
    return;
  }
  sector = fsbi->Data_firstsector =
      (finode->dentry_location - 2) * fsbi->sector_per_cluster;

  buf = (struct FAT32_Directory *)kmalloc(fsbi->bytes_per_cluster, 0);
  memset(buf, 0, fsbi->bytes_per_cluster);

  IDE_device_operation.transfer(ATA_READ, sector, fsbi->bytes_per_cluster,
                                (unsigned char *)buf);
  fdentry = buf + finode->dentry_position; // 刚读出来的待修改的数据

  /*  对旧数据进行修改  */
  fdentry->Dir_FileSize = inode->file_size;
  fdentry->Dir_FatClusLO = (finode->first_cluster & 0xffff);
  fdentry->Dir_FatClusHI =
      (fdentry->Dir_FatClusHI & 0xf000) | (finode->first_cluster >> 16);

  IDE_device_operation.transfer(ATA_WRITE, sector, fsbi->sector_per_cluster,
                                (unsigned char *)buf);
  kfree(buf);
}

/*  将修改后的superblock结构写回到硬盘中  */
void fat32_write_superblock(struct super_block *sb) {}

long FAT32_compare(struct dir_entry *parent_dentry, char *source_filename,
                   char *destination_filename) {}

long FAT32_hash(struct dir_entry *parent_dentry, char *filename) {}

long FAT32_release(struct dir_entry *dentry) {}

long FAT32_iput(struct dir_entry *dentry, struct index_node *inode) {}

long FAT32_mkdir(struct index_node *inode, struct dir_entry *dentry, int mode) {

}

long FAT32_rmdir(struct index_node *inode, struct dir_entry *dentry) {}

long FAT32_rename(struct index_node *old_inode, struct dir_entry *old_dentry,
                  struct index_node *new_inode, struct dir_entry *new_dentry) {}

long FAT32_getattr(struct dir_entry *dentry, unsigned long *attr) {}

long FAT32_setattr(struct dir_entry *dentry, unsigned long *attr) {}

long FAT32_open(struct index_node *inode, struct file *filep) { return 1; }

long FAT32_close(struct index_node *inode, struct file *filep) { return 1; }

long FAT32_read(struct file *filep, unsigned char *buf, unsigned long count,
                long *position) {
  struct FAT32_inode_info *finode =
      filep->dentry->dir_inode->private_index_info;
  struct FAT32_sb_info *fsbi = filep->dentry->dir_inode->sb->private_sb_info;

  unsigned long cluster = finode->first_cluster;
  unsigned long sector = 0;
  int length = 0;
  long retval = 0;
  int index = *position / fsbi->bytes_per_cluster; // 实际可读取的数据长度
  int offset = *position % fsbi->bytes_per_cluster;
  char *buffer = (char *)kmalloc(sizeof(fsbi->bytes_per_cluster), 0);
  if (!cluster) {
    return -EFAULT;
  }
  for (int i = 0; i < index; i++) {
    cluster = DISK1_FAT32_read_FAT_Entry(fsbi, cluster);
  }
  if (*position + count > filep->dentry->dir_inode->file_size) {

  } else {
    index = count;
  }

  return retval;
}

long FAT32_write(struct file *filep, unsigned char *buf, unsigned long count,
                 long *position) {
  return 1;
}

long FAT32_lseek(struct file *filep, long offset, long origin) {
  struct index_node *inode = filep->dentry->dir_inode;
  long pos = 0;
  switch (origin) {
  case SEEK_SET:
    pos = offset;
    break;
  case SEEK_CUR:
    pos = filep->position + offset;
    break;
  case SEEK_END:
    pos = filep->dentry->dir_inode->file_size + offset;
    break;
  default:
    return -EINVAL;
    break;
  }
  if (pos < 0 || pos > filep->dentry->dir_inode->file_size) {
    return EOVERFLOW;
  }
  filep->position = pos;
  return pos;
}

long FAT32_ioctl(struct index_node *inode, struct file *filep,
                 unsigned long cmd, unsigned long arg) {
  return 1;
}

struct super_block_operations FAT32_sb_ops = {
    .put_superblock = fat32_put_superblock,
    .write_superblock = fat32_write_superblock,
    .write_inode = fat32_write_inode,
};

struct dir_entry_options FAT32_dentry_ops = {.compare = FAT32_compare,
                                             .hash = FAT32_hash,
                                             .release = FAT32_release,
                                             .iput = FAT32_iput};

struct index_node_operations FAT32_inode_ops = {.create = FAT32_create,
                                                .lookup = FAT32_lookup,
                                                .mkdir = FAT32_mkdir,
                                                .getattr = FAT32_getattr,
                                                .setattr = FAT32_setattr,
                                                .rename = FAT32_rename};

struct file_operations FAT32_file_ops = {.open = FAT32_open,
                                         .close = FAT32_close,
                                         .read = FAT32_read,
                                         .write = FAT32_write,
                                         .lseek = FAT32_lseek,
                                         .ioctl = FAT32_ioctl};

/*  从FAT32中获取信息并构造超级块  */
struct super_block *
fat32_read_superblock(struct Disk_Partition_Table_Entry *DPTE, void *buf) {
  struct super_block *sbp = NULL; // 超级块
  struct FAT32_inode_info *finode = NULL;
  struct FAT32_BootSector *fbs = NULL; // 引导扇区
  struct FAT32_sb_info *fsbi = NULL;

  /*  fat32超级块  */
  sbp = (struct super_block *)kmalloc(sizeof(struct super_block), 0);
  memset(sbp, 0, sizeof(struct super_block));

  sbp->sb_ops = &FAT32_sb_ops;
  sbp->private_sb_info =
      (struct FAT32_sb_info *)kmalloc(sizeof(struct FAT32_sb_info), 0);
  memset(sbp->private_sb_info, 0, sizeof(struct FAT32_sb_info));

  /*  fat32引导扇区  */
  fbs = (struct FAT32_BootSector *)buf;
  fsbi = sbp->private_sb_info;
  fsbi->start_sector = DPTE->start_sector;
  fsbi->sector_count = DPTE->sectors_limit;
  fsbi->sector_per_cluster = fbs->BPB_SecPerClus;
  fsbi->bytes_per_cluster = fbs->BPB_SecPerClus * fbs->BPB_BytesPerSec;
  fsbi->bytes_per_sector = fbs->BPB_BytesPerSec;

  /*
   * 数据区的第一个扇区
   * 第一个分区的起始扇区 保留扇区 FAT表1 FAT表2 数据区
   */
  fsbi->Data_firstsector = DPTE->start_sector + fbs->BPB_RsvdSecCnt +
                           fbs->BPB_FATSz32 * fbs->BPB_NumFATs;
  fsbi->FAT1_firstsector = DPTE->start_sector + fbs->BPB_RsvdSecCnt;
  fsbi->sector_per_FAT = fbs->BPB_FATSz32;
  fsbi->NumFATs = fbs->BPB_NumFATs;
  fsbi->fsinfo_sector_infat = fbs->BPB_FSInfo;
  fsbi->bootsector_bk_infat = fbs->BPB_BKBootSec;

  fsbi->fat_fsinfo =
      (struct FAT32_FSInfo *)kmalloc(sizeof(struct FAT32_FSInfo), 0);
  memset(fsbi->fat_fsinfo, 0, 512);
  IDE_device_operation.transfer(ATA_READ, DPTE->start_LBA + fbs->BPB_FSInfo, 1,
                                (unsigned char *)fsbi->fat_fsinfo);

  /*  目录项  */
  sbp->root = (struct dir_entry *)kmalloc(sizeof(struct dir_entry), 0);
  memset(sbp->root, 0, sizeof(struct dir_entry));

  list_init(&sbp->root->child_node);
  list_init(&sbp->root->subdirs_list);
  sbp->root->parent = sbp->root;
  sbp->root->dir_ops = &FAT32_dentry_ops;
  sbp->root->name = (char *)kmalloc(2, 0);
  sbp->root->name[0] = '/';
  sbp->root->name_length = 1;

  /*  index node  */
  sbp->root->dir_inode =
      (struct index_node *)kmalloc(sizeof(struct index_node), 0);
  memset(sbp->root->dir_inode, 0, sizeof(struct index_node));
  sbp->root->dir_inode->inode_ops = &FAT32_inode_ops;
  sbp->root->dir_inode->f_ops = &FAT32_file_ops;
  sbp->root->dir_inode->file_size = 0;
  sbp->root->dir_inode->blocks =
      (sbp->root->dir_inode->file_size + fsbi->bytes_per_cluster - 1) /
      fsbi->bytes_per_cluster; // 该文件需要占用多少个簇(向上取整)
  sbp->root->dir_inode->attribute = FS_ATTR_DIR;
  sbp->root->dir_inode->sb = sbp;

  /*  fat32 root inode  */
  sbp->root->dir_inode->private_index_info =
      (struct FAT32_inode_info *)kmalloc(sizeof(struct FAT32_inode_info), 0);
  memset(sbp->root->dir_inode->private_index_info, 0,
         sizeof(struct FAT32_inode_info));
  finode = (struct FAT32_inode_info *)sbp->root->dir_inode->private_index_info;
  finode->first_cluster = fbs->BPB_RootClus;
  finode->dentry_location = 0;
  finode->dentry_position = 0;
  finode->create_date = 0;
  finode->create_time = 0;
  finode->write_date = 0;
  finode->write_time = 0;

  return sbp;
}

long FAT32_create(struct index_node *inode, struct dir_entry *dentry) {
  return 1;
}

/*
 * 从目录项中搜索出目标子目录项,搜索完成后对子目录项进行初始化
 */
struct dir_entry *FAT32_lookup(struct index_node *parent_inode,
                               struct dir_entry *destination_dentry) {
  int j;
  unsigned int cluster = 0;
  unsigned long sector = 0;
  unsigned char *buf = NULL;
  struct FAT32_inode_info *finode = parent_inode->private_index_info;
  struct FAT32_sb_info *fsbi = parent_inode->sb->private_sb_info;

  struct FAT32_Directory *tmpdentry = NULL;
  struct FAT32_LongDirectory *tmpldentry = NULL;
  struct index_node *p = NULL;

  buf = kmalloc(fsbi->bytes_per_cluster, 0);
  cluster = finode->first_cluster;

  /*
   * 先从起始簇号开始查起,一个目录不一定只有一个簇号
   * 也许当前目录的子目录和文件有很多,需要多个簇才能装得下
   * FAT32不区分根目录区和数据区,因为不需要做区分(这样能够让根目录区动态增长,使二者平衡)
   * 如果把根目录区占用的簇数量写死,那么很容易导致文件系统无法装下大量小文件的情况
   */
next_cluster:
  // 通过起始簇号找到对应的扇区
  sector = fsbi->Data_firstsector + (cluster - 2) * fsbi->sector_per_cluster;
  /*  读取该目录的起始簇号指向的一个簇的数据  */
  if (!IDE_device_operation.transfer(ATA_READ, sector, fsbi->sector_per_cluster,
                                     buf)) {
    log_to_screen(ERROR, "[error] FAT32 FS(lookup) read disk ERROR!");
    kfree(buf);
    return NULL;
  }
  tmpdentry = (struct FAT32_Directory *)buf;

  /*  遍历该目录的起始簇号对应的簇里面的所有根目录项  */
  for (int i = 0; i < fsbi->bytes_per_cluster; i += 32, tmpdentry++) {
    if (tmpdentry->Dir_Attr == ATTR_LONG_NAME) {
      continue;
    }
    if (tmpdentry->Dir_Name[0] == 0xe5 || tmpdentry->Dir_Name[0] == 0x05 ||
        tmpdentry->Dir_Name[0] == 0x00 // 无效的目录项
    ) {
      continue;
    }

    tmpldentry = (struct FAT32_LongDirectory *)tmpdentry - 1;
    j = 0; // j表示目标目录项文件名的索引,因此需要经常清零
    while (tmpldentry->LDIR_Attr == ATTR_LONG_NAME &&
           tmpldentry->LDIR_Ord != 0xe5) {
      for (int x = 0; x < 5; x++) { // 比较Name1
        if (j > destination_dentry->name_length &&
            tmpldentry->LDIR_Name1[x] == 0xffff) {
          continue;
        } else if (j > destination_dentry->name_length ||
                   tmpldentry->LDIR_Name1[x] !=
                       (unsigned short)destination_dentry->name[j++]) {
          goto continue_cmp_fail;
        }
      }
      for (int x = 0; x < 6; x++) { // 比较Name2
        if (j > destination_dentry->name_length &&
            tmpldentry->LDIR_Name2[x] == 0xffff) {
          continue;
        } else if (j > destination_dentry->name_length ||
                   tmpldentry->LDIR_Name2[x] !=
                       (unsigned short)destination_dentry->name[j++]) {
          goto continue_cmp_fail;
        }
      }
      for (int x = 0; x < 2; x++) { // 比较Name3
        if (j > destination_dentry->name_length &&
            tmpldentry->LDIR_Name3[x] == 0xffff) {
          continue;
        } else if (j > destination_dentry->name_length ||
                   tmpldentry->LDIR_Name3[x] !=
                       (unsigned short)destination_dentry->name[j++]) {
          goto continue_cmp_fail;
        }
      }

      if (j >= destination_dentry->name_length) {
        goto find_lookup_success;
      }

      tmpldentry--;
    }

    // 短文件基础名匹配
    j = 0;
    for (int x = 0; x < 8; x++) {
      switch (tmpdentry->Dir_Name[x]) {
      case ' ':
        /*
         * 在文件名和目录名里遇到 . 符号的处理策略不同
         * 如果是文件名里遇到.说明这是基础名和扩展名之间的分隔符
         * 如果是目录名里遇到.那只是单纯的有一个点而不是分隔符
         */
        if (!(tmpdentry->Dir_Attr & ATTR_DIRECTORY)) {
          if (destination_dentry->name[j] == '.') {
            continue; // 目标文件名遇到了点说明是基础名已经结束了,在表项里的基础名的末尾都是空格
          } else if (j < destination_dentry->name_length &&
                     tmpdentry->Dir_Name[x] == destination_dentry->name[j]) {
            j++; // 文件名里就是有空格
            break;
          } else if (j > destination_dentry->name_length) {
            continue;
          } else {
            break;
          }
        } else {
          if (j < destination_dentry->name_length &&
              tmpdentry->Dir_Name[x] == destination_dentry->name[j]) {
            j++;
            break;
          } else if (j == destination_dentry->name_length) {
            continue;
          } else {
            goto continue_cmp_fail;
          }
        }
      case 'A' ... 'Z':
      case 'a' ... 'z':
        if (tmpdentry->Dir_NTRes & LOWERCASE_BASE) {
          j++;
          break;
        } else {
          goto continue_cmp_fail;
        }
      case 0 ... 9:
        if (j < destination_dentry->name_length &&
            tmpdentry->Dir_Name[x] == destination_dentry->name[j]) {
          j++; // 如果一样,则比较下一个字符
          break;
        } else { // 否则就是匹配失败了
          goto continue_cmp_fail;
        }
      default:
        j++;
        break;
      }
    }

    // 匹配扩展名
    if (!(tmpdentry->Dir_Attr & ATTR_DIRECTORY)) {
      j++;
      for (int x = 8; x < 11; x++) {
        switch (tmpdentry->Dir_Name[x]) {
        case 'A' ... 'Z':
        case 'a' ... 'z':
          if (tmpdentry->Dir_NTRes & LOWERCASE_EXT) { // 基础名小写
            if (tmpdentry->Dir_Name[x] + 32 == destination_dentry->name[j]) {
              j++;
              break;
            } else {
              goto continue_cmp_fail;
            }
          } else {
            if (tmpdentry->Dir_Name[x] == destination_dentry->name[j]) {
              j++;
              break;
            } else {
              goto continue_cmp_fail;
            }
          }
        case 0 ... 9:
          if (tmpdentry->Dir_Name[x] == destination_dentry->name[j]) {
            j++;
            break;
          } else {
            goto continue_cmp_fail;
          }
        case ' ':
          if (j < destination_dentry->name_length &&
              tmpdentry->Dir_Name[x] == destination_dentry->name[j]) {
            j++;
            break;
          } else if (j > destination_dentry->name_length) {
            continue;
          } else {
            goto continue_cmp_fail;
          }

        default:
          goto continue_cmp_fail;
        }
      }
    }
  continue_cmp_fail:;
  }

  // 根据当前簇号从FAT表里获取对应的表项的值
  cluster = DISK1_FAT32_read_FAT_Entry(fsbi, cluster);
  if (getFATEntryType(cluster) != FAT_using_end) {
    goto next_cluster; // 如果不是最后一项那么继续查找
  }
  kfree(buf); // 注意释放
  return NULL;

find_lookup_success: // 查找成功
  /*  初始化目录项的inode信息  */
  p = (struct index_node *)kmalloc(sizeof(struct index_node), 0);
  memset(p, 0, sizeof(struct index_node));
  p->file_size = tmpdentry->Dir_FileSize;
  p->attribute =
      tmpdentry->Dir_Attr & ATTR_DIRECTORY ? FS_ATTR_DIR : FS_ATTR_FILE;
  p->blocks =
      (p->file_size + fsbi->bytes_per_cluster - 1) / fsbi->bytes_per_cluster;
  p->sb = parent_inode->sb; // 它们都属于同一个文件系统,因此superblock是一样的
  p->f_ops = &FAT32_file_ops;
  p->inode_ops = &FAT32_inode_ops;

  /*  初始化目录项的inode的私有信息  */
  p->private_index_info =
      (struct FAT32_inode_info *)kmalloc(sizeof(struct FAT32_inode_info), 0);
  memset(p->private_index_info, 0, sizeof(struct FAT32_inode_info));
  finode = p->private_index_info; // 单独把它拿出来方便操作
  finode->first_cluster =
      (tmpdentry->Dir_FatClusHI << 16 | tmpdentry->Dir_FatClusLO & 0xffffffff);
  finode->dentry_location = cluster;
  finode->dentry_position = tmpdentry - (struct FAT32_Directory *)buf;
  finode->create_date = tmpdentry->Dir_CrtDate;
  finode->write_date = tmpdentry->Dir_WrtDate;
  finode->create_time = tmpdentry->Dir_CrtTime;
  finode->write_time = tmpdentry->Dir_CrtTime;

  if ((tmpdentry->Dir_FatClusHI >> 12) && (p->attribute & FS_ATTR_FILE)) {
    p->attribute |= FS_ATTR_DEVICE;
  }

  destination_dentry->dir_inode = p;
  kfree(buf);
  return destination_dentry;
}
