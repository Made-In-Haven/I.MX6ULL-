#include "bsp_rtc.h"


/*rtc初始化函数*/
void rtc_init(struct rtc_dateTime * rtc_date)
{
    SNVS->HPCOMR |= (1<<31)|(1<<8);
    
    //设置现在的时间 
    rtc_setDayTime(rtc_date);

    rtc_enable();

}

/*时间转换函数，将年月日时分秒转化为秒数*/
u64 rtc_coverdate_to_seconds(struct rtc_dateTime *datetime)
{	
	unsigned short i = 0;
	u64 seconds = 0;
	unsigned int days = 0;
	unsigned short monthdays[] = {0U, 0U, 31U, 59U, 90U, 120U, 151U, 181U, 212U, 243U, 273U, 304U, 334U};
	
	for(i = 1970; i < datetime->year; i++)
	{
		days += DAYS_IN_A_YEAR; 		/* 平年，每年365天 */
		if(rtc_isleapyear(i)) days += 1;/* 闰年多加一天 		*/
	}

	days += monthdays[datetime->month];
	if(rtc_isleapyear(i) && (datetime->month >= 3)) days += 1;/* 闰年，并且当前月份大于等于3月的话加一天 */

	days += datetime->day - 1;

	seconds = (u64)days * SECONDS_IN_DAY + 
				datetime->hour * SECONDS_IN_HOUR +
				datetime->minute * SECONDS_IN_MINUTE +
				datetime->second;

	return seconds;	
}

/*
 * @description	: 判断指定年份是否为闰年，闰年条件如下:
 * @param - year: 要判断的年份
 * @return 		: 1 是闰年，0 不是闰年
 */
unsigned char rtc_isleapyear(unsigned short year)
{	
	unsigned char value=0;
	
	if(year % 400 == 0)
		value = 1;
	else 
	{
		if((year % 4 == 0) && (year % 100 != 0))
			value = 1;
		else 
			value = 0;
	}
	return value;
}


/*使能rtc*/
void rtc_enable()
{
    SNVS->LPCR |= (1<<0);

    // while(((SNVS->LPCR)&(0x1))==0);
}

/*关闭RTC*/
void rtc_disable()
{
    SNVS->LPCR &= ~(1<<0);

    // while(((SNVS->LPCR)&(0x1))==1);
}

/*rtc函数，时间设置函数*/
void rtc_setDayTime(struct rtc_dateTime *date_time)
{
    unsigned temp = SNVS->LPCR;

    rtc_disable();

    u64 seconds = rtc_coverdate_to_seconds(date_time);

	SNVS->LPSRTCMR = (unsigned int)(seconds >> 17); /* 设置高16位 */
	SNVS->LPSRTCLR = (unsigned int)(seconds << 15); /* 设置地16位 */

    if((temp & 0x1) == 1)
    {
        rtc_enable();
    }



    
}

/*rtc函数，读取秒数*/
unsigned long long rtc_read_second()
{
    unsigned long long seconds = 0;
	seconds = (SNVS->LPSRTCMR << 17) | (SNVS->LPSRTCLR >> 15);
    return seconds;

}

/*rtc函数，获取年月日*/
void rtc_getDateTime(struct rtc_dateTime *datetime)
{
    unsigned long long second = rtc_read_second();
    rtc_convertseconds_to_datetime(second, datetime);
}

/*秒数转年月日*/
void rtc_convertseconds_to_datetime(u64 seconds, struct rtc_dateTime *datetime)
{
    u64 x;
    u64  secondsRemaining, days;
    unsigned short daysInYear;

    /* 每个月的天数       */
    unsigned char daysPerMonth[] = {0U, 31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U};

    secondsRemaining = seconds; /* 剩余秒数初始化 */
    days = secondsRemaining / SECONDS_IN_DAY + 1; 		/* 根据秒数计算天数,加1是当前天数 */
    secondsRemaining = secondsRemaining % SECONDS_IN_DAY; /*计算天数以后剩余的秒数 */

	/* 计算时、分、秒 */
    secondsRemaining = secondsRemaining % SECONDS_IN_HOUR;
    datetime->minute = secondsRemaining / 60;
    datetime->second = secondsRemaining % SECONDS_IN_MINUTE;

    /* 计算年 */
    daysInYear = DAYS_IN_A_YEAR;
    datetime->year = YEAR_RANGE_START;
    while(days > daysInYear)
    {
        /* 根据天数计算年 */
        days -= daysInYear;
        datetime->year++;

        /* 处理闰年 */
        if (!rtc_isleapyear(datetime->year))
            daysInYear = DAYS_IN_A_YEAR;
        else	/*闰年，天数加一 */
            daysInYear = DAYS_IN_A_YEAR + 1;
    }
	/*根据剩余的天数计算月份 */
    if(rtc_isleapyear(datetime->year)) /* 如果是闰年的话2月加一天 */
        daysPerMonth[2] = 29;

    for(x = 1; x <= 12; x++)
    {
        if (days <= daysPerMonth[x])
        {
            datetime->month = x;
            break;
        }
        else
        {
            days -= daysPerMonth[x];
        }
    }

    datetime->day = days;

}
