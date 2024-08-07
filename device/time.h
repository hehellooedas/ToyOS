#ifndef __DEVICE_TIME_H
#define __DEVICE_TIME_H


#include <io.h>
#include <interrupt.h>
#include <printk.h>



/*
 * RTC实时时钟,记录真实世界里的时间,存储在CMOS存储区中
 * CMOS是记录各项硬件参数且嵌入在主板上面的寄存器
 * CMOS存储器是一个位数极少的低电压静态内存芯片,由纽扣电池提供外部供电
 */



struct time{
    int second;
    int minute;
    int hour;
    int day;
    int mounth;
    int year;
};



#define PORT_CMOS_INDEX     0x70    //索引端口
#define PORT_CMOS_DATA      0x71    //数据端口


#define RTC_SECOND_INDEX    0x00
#define RTC_MINUTE_INDEX    0x02
#define RTC_HOUR_INDEX      0x04
#define RTC_DAY_INDEX       0x07
#define RTC_MOUTH_INDEX     0x08
#define RTC_YEAR_INDEX      0x09
#define RTC_CENTRY_INDEX    0X32


#define CMOS_READ(addr) ({               \
    out8(PORT_CMOS_INDEX, 0x80 | addr);  \
    in8(PORT_CMOS_DATA);                 \
})


void get_cmos_time(struct time* time);
void print_current_time(void);



#endif // !__DEVICE_TIME_H
