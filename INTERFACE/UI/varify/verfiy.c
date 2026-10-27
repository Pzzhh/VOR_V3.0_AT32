#include "control.h"
#include "../control/control_Hardware_API.h"
#include "../../commuication/communication_API.h"
#include "../other/system_function.h"
#include "stdint.h"
#include "../other/meassage_center.h"
#include "stdarg.h"

#ifndef ATM32F43_xx
#include "Windows.h"
#include <stdio.h>
#else
#include "FreeRTOS.h"
#include "task.h"
#endif // use_windows
#include "verfiy.h"
#include "../control/control_Hardware_API.h"
#include "../control/control.h"
#define Using_UI_LOCK 1

#define use_windows
#ifdef ATM32F43_xx
#else
typedef uint32_t TaskHandle_t;

#endif
timetyped InfiniteTag = { .date.year = 99,
						.date.month = 1,
.date.date = 1,
.date.hour = 1,
.date.min = 1,
.date.sec = 1 };


const timetyped HardResetvaildtag = { .date.year = 2026,
					.date.month = 9,
					.date.date = 26,
					.date.hour = 1,
					.date.min = 1,
					.date.sec = 1 };
//timetyped LocalTimeTag = { .date.year = 26,
//						.date.month = 1,
//.date.date = 1,
//.date.hour = 1,
//.date.min = 1,
//.date.sec = 1 };

static int get_month_days(int year, int month);

#ifdef ATM32F43_xx
#define VERIFY_LOCK() taskENTER_CRITICAL()
#define VERIFY_UNLOCK() taskEXIT_CRITICAL()
#else
static SRWLOCK verify_lock = SRWLOCK_INIT;
#define VERIFY_LOCK() AcquireSRWLockExclusive(&verify_lock)
#define VERIFY_UNLOCK() ReleaseSRWLockExclusive(&verify_lock)
#endif

extern Task_control_info control_info;
Verify_Cache_Type Verify_Cache = {0};

static uint8_t Verify_DateValid(const timetyped *t)
{
    return t->date.year >= 2000 && t->date.year <= 2099 &&
        t->date.month >= 1 && t->date.month <= 12 &&
        t->date.date >= 1 &&
        t->date.date <= get_month_days(t->date.year, t->date.month) &&
        t->date.hour < 24 && t->date.min < 60 && t->date.sec < 60;
}

uint8_t Verify_Compare_time(timetyped *expiredTag, timetyped localtime)
{
    if (!Verify_DateValid(&localtime))
        return SetTimeOnly;
    if (memcmp(expiredTag, &InfiniteTag, sizeof(InfiniteTag)) == 0)
        return okay;
    if (!Verify_DateValid(expiredTag))
        return NoTag;
    for (int i = 0; i < 6; ++i)
    {
        if (localtime.num[i] < expiredTag->num[i]) return okay;
        if (localtime.num[i] > expiredTag->num[i]) return expired;
    }
    return expired;
}

void Verify_Config_Update(uint8_t id, const uint8_t *data, int length)
{
    timetyped date = {0};
    uint8_t valid = 0;
    if (id == 1 && data && length == sizeof(date))
    {
        memcpy(&date, data, sizeof(date));
        valid = Verify_DateValid(&date) ||
            memcmp(&date, &InfiniteTag, sizeof(date)) == 0;
    }
    VERIFY_LOCK();
    if (id == 1)
    {
        Verify_Cache.licenseDate = date;
        Verify_Cache.licenseValid = valid;
    }
    else if (id == 3)
        Verify_Cache.timeConfigured = data && length == 2 &&
            data[0] == 0xAA && data[1] == 0xAA;
    VERIFY_UNLOCK();
}

void Verify_Init(void)
{
    uint8_t buf[50] = {0};
    HAL_CONFIG_READ(1, buf, sizeof(buf));
    HAL_CONFIG_READ(3, buf, sizeof(buf));
    Verify_Check_isExpired();
}

static int is_leap_year(int year) {
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// 获取某年某月的天数
static int get_month_days(int year, int month) {
	static const int days[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
	if (month == 2 && is_leap_year(year)) {
		return 29;
	}
	return days[month - 1];
}

// 将日期转换为自某一基准日以来的总天数
static int date_to_days(const timetyped* t) {
	int y = t->date.year;
	int m = t->date.month;
	int d = t->date.date;
	int days = 0;

	// 计算年份的天数
	for (int i = 0; i < y; ++i) {
		days += is_leap_year(i) ? 366 : 365;
	}
	// 计算月份的天数
	for (int i = 1; i < m; ++i) {
		days += get_month_days(y, i);
	}
	// 加上日期
	days += d;
	return days;
}

void Verify_TimeCheck(timetyped *time)
{
    if (time->date.month == 0 || time->date.month > 12)
        time->date.month = 12;
    int days = get_month_days(time->date.year, time->date.month);
    if (time->date.date > days) time->date.date = days;
    if (time->date.date == 0) time->date.date = 1;
}

// 计算 expiredTag 距离 localtime 剩余天数（正数为剩余，负数为已过期）
int Verify_Calc_RemainDays(const timetyped* expiredTag, const timetyped* localtime)
{
	int days_expired = date_to_days(expiredTag);
	int days_local = date_to_days(localtime);
	return days_expired - days_local;
}

void Verify_hardResetVaild(void)
{

	timetyped localtime = { 0 };
	HAL_TIME_Get(&localtime);
	uint8_t buf[50] = { 0 };
	uint8_t res = HAL_CONFIG_READ(1, buf, 0x20);
	// 自动设置 设备有效期

	if (res != 0)
	{
		timetyped* expiredTag = (timetyped*)&buf[0];
		if (Verify_Calc_RemainDays(&HardResetvaildtag, &localtime) > 0)
		{
			if (memcmp(expiredTag, &HardResetvaildtag, sizeof(HardResetvaildtag)) != 0)
			{
				memcpy(expiredTag, &HardResetvaildtag, sizeof(HardResetvaildtag));
				Verify_TimeCheck(expiredTag);
				HAL_CONFIG_WRITE(1, (uint8_t*)expiredTag, sizeof(timetyped));
			}
		}
	}
}

uint8_t Verify_Check_isExpired(void)
{
    timetyped now = {0};
    uint8_t status;
    VERIFY_LOCK();
    if (!Verify_Cache.timeConfigured)
        status = SeTboth;
    else if (!Verify_Cache.licenseValid)
        status = NoTag;
    else if (!HAL_TIME_Get(&now))
        status = SetTimeOnly;
    else
        status = Verify_Compare_time(&Verify_Cache.licenseDate, now);
    control_info.isExpired = status;
    VERIFY_UNLOCK();
    return status;
}

//0 infinite
uint8_t Verify_get_license_Date(timetyped* t)
{

	uint8_t buf[0x20] = { 0 };
	uint8_t res = HAL_CONFIG_READ(1, buf, 0x20);
	if (res == sizeof(timetyped))
	{
		memcpy(t, buf, sizeof(timetyped));
		return 1;
	}
	return 0;
}




timetyped Verify_Month_2_Expired_Date(uint8_t month)
{
	timetyped localtime = { 0 };
	HAL_TIME_Get(&localtime);
    uint16_t months = localtime.date.month - 1 + month;
    localtime.date.year += months / 12;
    localtime.date.month = months % 12 + 1;
    Verify_TimeCheck(&localtime);
    HAL_CONFIG_WRITE(1, (uint8_t*)&localtime, sizeof(localtime));
    return localtime;
}

uint8_t Verify_Set_Time(void* time)
{
	uint8_t buf[2] = { 0xAA,0xAA };
    if (!time || !Verify_DateValid((timetyped *)time) || !HAL_TIME_Set(time))
        return 0;
    uint8_t result = HAL_CONFIG_WRITE(3, buf, 2);
    Verify_Check_isExpired();
    return result;
}


uint8_t Verify_Factory_Reset(void)
{
	uint8_t buf[0x20] = { 0 };
	HAL_CONFIG_WRITE(1, (uint8_t*)buf, 0);
	HAL_CONFIG_WRITE(3, (uint8_t*)buf, 0);
    return 0;
}

uint8_t Verify_Set_Subscribe(uint8_t month)
{

	if (month == 0xFF)
	{
		HAL_CONFIG_WRITE(1, (uint8_t*)&InfiniteTag, sizeof(InfiniteTag));
		return 0;
	}
	Verify_Month_2_Expired_Date(month);
	return 0;
}



