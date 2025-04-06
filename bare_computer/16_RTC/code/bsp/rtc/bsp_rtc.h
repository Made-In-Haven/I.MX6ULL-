#ifndef __BSP_RTC_H
#define __BSP_RTC_H

#include "imx6ul.h"

/*和时间相关的宏定义*/
#define SECONDS_IN_DAY (86400)
#define SECONDS_IN_HOUR (3600)
#define SECONDS_IN_MINUTE (60)
#define DAYS_IN_A_YEAR (365)
#define YEAR_RANGE_START (1970)
#define YEAR_RANGE_END (2099)


/*定义和时间相关的结构体*/
struct rtc_dateTime{
    unsigned short year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
};

void rtc_disable();
void rtc_enable();
void rtc_init(struct rtc_dateTime * rtc_data);
u64 rtc_coverdate_to_seconds(struct rtc_dateTime *datetime);
unsigned char rtc_isleapyear(unsigned short year);
void rtc_setDayTime(struct rtc_dateTime *data_time);
unsigned long long rtc_read_second();
void rtc_getDateTime(struct rtc_dateTime *datetime);
void rtc_convertseconds_to_datetime(u64 seconds, struct rtc_dateTime *datetime);




#endif