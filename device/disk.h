#ifndef __DSEVICE_DISK_H
#define __DSEVICE_DISK_H


/*  磁盘0  */
#define PORT_DISK0_DATA         0x1f0  //数据端口
#define PORT_DISK0_ERR_FEATURE  0x1f1  //错误状态
#define PORT_DISK0_SECTOR_CNT   0x1f2  //要操作的扇区数
#define PORT_DISK0_SECTOR_LOW   0x1f3  //扇区号/LBA(7:0)
#define PORT_DISK0_SECTOR_MID   0x1f4  //柱面号(7:0)/LBA(15:8)
#define PORT_DISK0_SECTOR_HIGH  0x1f5  //柱面号(15:8)/LBA(13:16)
#define PORT_DISK0_DEVICE       0x1f6  //设备工作方式
#define PORT_DISK0_CMD          0x1f7  //控制命令端口
#define PORT_DISK0_ALT_STA_CTL  0x3f6



/*  磁盘1  */
#define PORT_DISK1_DATA         0x170
#define PORT_DISK1_ERR_FEATURE  0x171
#define PORT_DISK1_SECTOR_CNT   0x172  
#define PORT_DISK1_SECTOR_LOW   0x173 
#define PORT_DISK1_SECTOR_MID   0x174 
#define PORT_DISK1_SECTOR_HIGH  0x175 
#define PORT_DISK1_DEVICE       0x176     
#define PORT_DISK1_CMD          0x177
#define PORT_DISK1_ALT_STA_CTL  0x376



/*  设备状态  */
#define DISK_STATUS_BUSY    (1 << 7)  //磁盘在忙
#define DISK_STATUS_READY   (1 << 6)  //磁盘就绪、准备好了
#define DISK_STATUS_SEEK    (1 << 4)  //正在寻道
#define DISK_STATUS_REQ     (1 << 3)  //数据请求
#define DISK_STATUS_ERROR   (1 << 0)  //命令执行错误




#endif // !__DSEVICE_DISK_H