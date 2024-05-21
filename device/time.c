#include <printk.h>
#include <time.h>
#include <interrupt.h>



void get_cmos_time(struct time* time){
    enum intr_status old_status = intr_disable();
    do{
        time->year = CMOS_READ(RTC_YEAR_INDEX) + CMOS_READ(RTC_CENTRY_INDEX) * 0x100;
        time->mounth = CMOS_READ(RTC_MOUTH_INDEX);
        time->day = CMOS_READ(RTC_DAY_INDEX);
        time->hour = CMOS_READ(RTC_HOUR_INDEX);
        time->minute = CMOS_READ(RTC_MINUTE_INDEX);
        time->second = CMOS_READ(RTC_SECOND_INDEX);
    }while(time->second != CMOS_READ(0x00));
    out8(PORT_CMOS_INDEX,0x00 );
    set_intr_status(old_status);
}



void print_current_time(void){
    struct time current_time;
    get_cmos_time(&current_time);
    color_printk(GREEN,BLACK,"%x-%x-%x  %x-%x-%x\n",current_time.year,current_time.mounth,current_time.day,current_time.hour,current_time.minute,current_time.second);
}