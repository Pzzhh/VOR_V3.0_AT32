#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <setjmp.h>
#define ATM32F43_xx
#define Task_ARY_MAX 15
#include "INTERFACE/UI/control/control.h"
#include "INTERFACE/UI/varify/verfiy.h"
#define taskENTER_CRITICAL() ((void)0)
#define taskEXIT_CRITICAL() ((void)0)
#define portMAX_DELAY 0xffffffff
#define pdTRUE 1
#define Rk3588_Task_Cancel do { control->Rk3588_Flag.Lflag = Cancel; control->Rk3588_Flag.Rflag = Cancel; } while (0)
typedef void *SemaphoreHandle_t;
static int mutex_failed;
static SemaphoreHandle_t xSemaphoreCreateMutex(void) { return mutex_failed ? NULL : (void *)1; }
static int xSemaphoreTake(SemaphoreHandle_t s, unsigned long wait) { return 1; }
static void xSemaphoreGive(SemaphoreHandle_t s) {}
#define EROM_MAP_info_pS 32
#define EROM_MAP_info_base 0
#define EROM_MAP_Task_Stroage 0x300
static uint8_t eeprom[0x300];
static int read_failed, write_failed, read_calls, task_loads, movements, zero_calls;
static timetyped current_time;
static int rtc_failed;
Task_control_info control_info;
static Task_control_info *control;
Task_Parameter_Struct TaskArray[Task_ARY_MAX];
static jmp_buf handler_exit;
static int idle_calls;
static int expire_pending;
static timetyped pending_time;
static uint16_t crc16_modbus(const uint8_t *data, unsigned int length)
{
    uint16_t crc = 0xffff;
    while (length--) {
        crc ^= *data++;
        for (int i = 0; i < 8; ++i) crc = (crc >> 1) ^ ((crc & 1) ? 0xa001 : 0);
    }
    return crc;
}
static int M24C64_Read_Byte(uint16_t addr, uint8_t *buf, uint16_t len)
{
    ++read_calls;
    if (read_failed) return 1;
    memcpy(buf, eeprom + addr, len);
    return 0;
}
static int M24C64_Write_Byte(uint16_t addr, uint8_t *buf, uint16_t len)
{
    if (write_failed) return 1;
    memcpy(eeprom + addr, buf, len);
    return 0;
}
uint8_t HAL_TIME_Get(timetyped *t) { *t = current_time; return !rtc_failed; }
uint8_t HAL_TIME_Set(timetyped *t) { current_time = *t; return 1; }
uint16_t HAL_Task_GetArray(Task_Parameter_Struct *array, uint16_t size)
{ ++task_loads; return 1; }
uint8_t Ctrl_Msg_Printf(const char *format, ...)
{
    va_list args; va_start(args, format);
    vsnprintf((char *)control_info.message, sizeof(control_info.message), format, args);
    va_end(args); return 1;
}
static void SaftExitDelay(int delay, int ignored)
{
    if (++idle_calls == 1) {
        control_info.State_Bit.isRun = 1;
        if (expire_pending) current_time = pending_time;
    } else longjmp(handler_exit, 1);
}
static void ZEROControlFunction(int start) { ++zero_calls; longjmp(handler_exit, 1); }
static void VorControlFunction(Task_Parameter_Struct *t, Task_control_info *e) { ++movements; }
static void ContControlFunction(Task_Parameter_Struct *t, Task_control_info *e) { ++movements; }
static void OvarControlFunction(Task_Parameter_Struct *t, Task_control_info *e, int retract) { ++movements; }
static void TcControlFunction(Task_Parameter_Struct *t, Task_control_info *e) { ++movements; }
static void VHITControlFunction(Task_Parameter_Struct *t, Task_control_info *e) { ++movements; }
int HAL_Outage_Log(uint8_t flag, uint8_t index) { return 1; }
uint8_t HAL_Beep_Enable(uint32_t ms) { return 1; }
static void task_interval_handle(int i) {}
static uint32_t ControlGetTick(void) { return 0; }
static void HAL_USB_Log_Printf(const char *format, ...) {}
uint8_t Message_Center_Send_prinft(const char *name, void *a, uint8_t size, const char *format, ...) { return 0; }
timetyped InfiniteTag = { .date.year = 99,
						.date.month = 1,
.date.date = 1,
.date.hour = 1,
.date.min = 1,
.date.sec = 1 };


const timetyped HardResetvaildtag = { .date.year = 2026,
					.date.month = 9,
					.date.date = 19,
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



static int HAL_CONFIG_READ_RAW(uint8_t ID, uint8_t *buffer, uint32_t size)
{
#ifndef ATM32F43_xx
	int res = 0;
	char Path[1000];
	uint8_t Rbuf[500];
	_getcwd(Path, 1000);
	int pos = strlen(Path);
	sprintf(&Path[pos], "\\config.hex");
	FILE *f = fopen(Path, "rb");
	if (f == NULL)
	{
		f = fopen(Path, "wb");
		if (f == NULL)
			return res;
	}
	int readcount = fread(Rbuf, 500, 1, f);
	fclose(f);
	if (readcount != 0)
	{
		int count = 0;
		for (int i = 0; i < 500; i += 50)
		{
			if (Rbuf[i] == ID)
			{
				uint32_t crc_Len = Rbuf[1 + i];
                if (crc_Len > 46) return 0;
				uint16_t Crc = crc16_modbus(&Rbuf[i + 2], crc_Len);
				uint16_t Filecrc = Rbuf[i + crc_Len + 2];
				Filecrc = Filecrc << 8;
				Filecrc = Filecrc + Rbuf[i + crc_Len + 3];
				if (Filecrc == Crc)
				{
					if (size >= crc_Len)
					{
						memcpy(buffer, &Rbuf[i + 2], crc_Len);
						res = crc_Len;
					}
					else
					{
						res = 0;
					}
				}
			}
			count++;
		}
	}

	// crc check
	return res;
#else
	if (ID >= (EROM_MAP_Task_Stroage - EROM_MAP_info_base) / EROM_MAP_info_pS)
		return 0;
	int res = 0;
	uint16_t crcValue;
	// uint8_t *BKSbuf = (uint8_t *)0x40024000 + ID * 20;
	uint8_t *BKSbuf = (uint8_t *)malloc(EROM_MAP_info_pS);
	if (BKSbuf == 0)
		return 0;
	memset(BKSbuf, 0, EROM_MAP_info_pS);
	M24C64_Read_Byte(ID * EROM_MAP_info_pS + EROM_MAP_info_base, BKSbuf, EROM_MAP_info_pS);
	int readcount = BKSbuf[1];
	uint16_t ReadEnd = readcount + 2;
	// data format:
	// first id len [...] crc
	if (BKSbuf[0] == ID)
	{
		if (readcount <= EROM_MAP_info_pS - 4 && (uint32_t)readcount <= size)
		{
			crcValue = crc16_modbus(BKSbuf, readcount + 2);
			uint16_t Ocrc = BKSbuf[ReadEnd];
			Ocrc = Ocrc * 256 + BKSbuf[ReadEnd + 1];
			if (crcValue == Ocrc)
			{
				res = readcount;
				memcpy(buffer, &BKSbuf[2], readcount);
			}
		}
	}
	free(BKSbuf);
	return res;
#endif
}

static uint8_t HAL_CONFIG_WRITE_RAW(uint8_t ID, uint8_t *buffer, uint32_t size)
{
#ifndef ATM32F43_xx
	char Path[1000];
	uint8_t buf[500] = {0};
	if (size > 46)
		return 0;
	_getcwd(Path, 1000);
	int pos = strlen(Path);
	sprintf(&Path[pos], "\\config.hex");
	FILE *f = fopen(Path, "r+b");
	if (f == 0)
		return 0;
	fread(buf, sizeof(buf), 1, f);
	fclose(f);
	f = fopen(Path, "w+b");
	if (f == 0)
		return 0;
	int count = 0;
	for (int i = 0; i < 500; i += 50)
	{
		if (count == ID)
		{
			uint16_t Crc = crc16_modbus(buffer, size);
			uint8_t crcHL[2] = {Crc >> 8, Crc & 0xff};
			buf[i] = ID;
			buf[i + 1] = size;
			memcpy(&buf[i + 2], buffer, size);
			memcpy(&buf[i + size + 2], crcHL, 2);
		}
		count++;
	}

	fwrite(buf, sizeof(buf), 1, f);
	fclose(f);
	return 1;
#else
	int res = 0;
	uint16_t crcValue;
	uint8_t *buf = (uint8_t *)malloc(EROM_MAP_info_pS);
	if (buf == 0)
		return 0;
    memset(buf, 0, EROM_MAP_info_pS);
	// uint8_t* buf = (uint8_t*)0x40024000 + ID * 20;
	//	int readcount = buf[1];
	int buf_end;
	// data format:
	// first id len [...] crc
	if (ID < (EROM_MAP_Task_Stroage - EROM_MAP_info_base) / EROM_MAP_info_pS &&
        size <= EROM_MAP_info_pS - 4)
	{
		if (size == 0)
		{
			memset(&buf[0], 0,EROM_MAP_info_pS);
			M24C64_Write_Byte(ID * EROM_MAP_info_pS + EROM_MAP_info_base, buf, EROM_MAP_info_pS);
			res = 1;
			goto out;
		}
		memcpy(&buf[2], buffer, size);
		buf[0] = ID;
		buf[1] = size;
		crcValue = crc16_modbus(buf, size + 2);
		buf_end = size + 2;
		buf[buf_end] = crcValue >> 8;
		buf[buf_end + 1] = crcValue & 0xff;
		M24C64_Write_Byte(ID * EROM_MAP_info_pS + EROM_MAP_info_base, buf, EROM_MAP_info_pS);
		res = 1;
	}
	out:
	free(buf);
	return res;
#endif
}

/* Serialize records and cache publication; never hold a critical section over I2C. */
#ifdef ATM32F43_xx



static SemaphoreHandle_t config_mutex;
static uint8_t Config_Lock(void)
{
    taskENTER_CRITICAL();
    if (!config_mutex) config_mutex = xSemaphoreCreateMutex();
    taskEXIT_CRITICAL();
    return config_mutex && xSemaphoreTake(config_mutex, portMAX_DELAY) == pdTRUE;
}
static void Config_Unlock(void)
{
    if (config_mutex) xSemaphoreGive(config_mutex);
}
#else
static SRWLOCK config_mutex = SRWLOCK_INIT;
#define Config_Lock() (AcquireSRWLockExclusive(&config_mutex), 1)
#define Config_Unlock() ReleaseSRWLockExclusive(&config_mutex)
#endif

int HAL_CONFIG_READ(uint8_t ID, uint8_t *buffer, uint32_t size)
{
    int result;
    if (!Config_Lock())
    {
        if (ID == 1 || ID == 3)
        {
            Verify_Config_Update(ID, 0, 0);
            Verify_Check_isExpired();
        }
        return 0;
    }
    result = HAL_CONFIG_READ_RAW(ID, buffer, size);
    if (ID == 1 || ID == 3)
    {
        Verify_Config_Update(ID, buffer, result);
        Verify_Check_isExpired();
    }
    Config_Unlock();
    return result;
}

uint8_t HAL_CONFIG_WRITE(uint8_t ID, uint8_t *buffer, uint32_t size)
{
    uint8_t result;
    if (!Config_Lock())
    {
        if (ID == 1 || ID == 3)
        {
            Verify_Config_Update(ID, 0, 0);
            Verify_Check_isExpired();
        }
        return 0;
    }
    if (ID == 1 || ID == 3) Verify_Config_Update(ID, 0, 0);
    result = HAL_CONFIG_WRITE_RAW(ID, buffer, size);
    if (ID == 1 || ID == 3)
    {
        uint8_t actual[50] = {0};
        int length = HAL_CONFIG_READ_RAW(ID, actual, sizeof(actual));
        Verify_Config_Update(ID, actual, length);
        Verify_Check_isExpired();
        if (size != length || (size && memcmp(buffer, actual, size) != 0))
            result = 0;
    }
    Config_Unlock();
    return result;
}

static uint8_t Task_control_LicenseAllowed(Task_control_info *e)
{
    uint8_t status = Verify_Check_isExpired();
    e->isExpired = status;
    if (status == okay) return 1;
    Ctrl_Msg_Printf(status == expired ? "Start rejected: license expired" :
        status == NoTag ? "Start rejected: invalid license" :
        "Start rejected: time not configured");
    return 0;
}
uint8_t Task_control_Begin(Task_control_info *e)
{
	if (!e) return 0;
	if (e->State_Bit.IsRunning == 1)
	{
		return 0;
	}
	if (!Task_control_LicenseAllowed(e))
    {
        e->StartWithIndex = 0;
        return 0;
    }
	e->taskCount = HAL_Task_GetArray(TaskArray, Task_ARY_MAX);
	if (e->taskCount == 0 || e->taskCount > Task_ARY_MAX)
		return 0;
	e->taskArray = TaskArray;
	e->State_Bit.flag = 0;
	e->State_Bit.isRun = 1;
	return 1;
}
void Task_control_handler(Task_control_info *e)
{
	uint8_t Rk3588_ReadyFlag_1 = 0, Rk3588_ReadyFlag_2 = 0;
	uint8_t skip_loop = 0;
	if (e == 0)
		return;
	control = e;
	e->State_Bit.isRun = 0;
	e->State_Bit.WaitRk = 0;
	Ctrl_Msg_Printf(" ");
	//	ZEROControlFunction(1);
	// HAL_Beep_Enable(500);
	// uint16_t len = 0;
BEGIN_POS:
	// get all task need to implement
	e->State_Bit.isRun = 0;
	e->State_Bit.IsRunning = 0;
	while (1)
	{
		if (e->State_Bit.isRun == 1)
        {
            if (!Task_control_LicenseAllowed(e))
            {
                e->State_Bit.isRun = 0;
                e->StartWithIndex = 0;
                continue;
            }
			e->State_Bit.flag = 0;
			e->Error_Bit.flag = 0;
			if (e == 0 || e->taskArray == 0)
			{
				e->Error_Bit.paraError = 1;
				continue;
			}
			break;
		}
		SaftExitDelay(100, 0);
	}

	e->State_Bit.IsRunning = 1;
	e->State_Bit.isPause = 0;
	e->Rk3588_Flag.WifiMsgFlag = 0;
	e->Rk3588_Flag.isWifiCauseStop = 0;
	e->currentCount = 0;
	skip_loop = 0;

	uint16_t task_size = e->taskCount;
	int i = 0;
	Ctrl_Msg_Printf(" ");
	HAL_Outage_Log(1, 0);
	if (e->StartWithIndex < task_size)
	{
		i = e->StartWithIndex;
		e->StartWithIndex = 0;
		if (i == 0)
			ZEROControlFunction(1);
	}
	else
	{
		e->StartWithIndex = 0;
		goto BEGIN_POS;
	}

	for (; i < task_size; i++)
	{
		HAL_Outage_Log(1, i);
		Task_Parameter_Struct *task = &e->taskArray[i];
		e->currentCount = i;
		e->UI_para.state = taskInv;

		uint8_t isretract = 1;
		switch (task->mode)
		{
		case Task_VOR:
			VorControlFunction(task, e);
			break;
		case Task_Continue:
			ContControlFunction(task, e);
			break;
		case Task_OVAR:
			isretract = 1;
			if (i < (task_size - 1) && e->taskArray[i + 1].mode == Task_OVAR)
				isretract = 0;
			OvarControlFunction(task, e, isretract);
			break;
		case Task_TC:
			TcControlFunction(task, e);
			break;
		case Task_VHIT:
			VHITControlFunction(task, e);
			break;
		default:
			break;
		}
		Rk3588_Send_RecordStop;
		e->State_Bit.isPause = 0;
		if (e->Dev_Mode_Bit.Zero == 0)
			ZEROControlFunction(0);
		Ctrl_Msg_Printf(" ");
		SaftExitDelay(1000, 0);
		Rk3588_Send_ReqRkErrorStatus;
		task_interval_handle(0);
		if (e->Rk3588_Flag.isUsingBlock)
		{
			if (e->Rk3588_Flag.WifiMsgFlag != 3)
			{
				uint8_t LE = e->Rk3588_Flag.LerrorCode, RE = e->Rk3588_Flag.RerrorCode;
				HAL_Beep_Enable(1000);
				Message_Center_Send_prinft("page1", 0,
										   0,
										   "NoticeWifierror %d %d %d %d", e->Rk3588_Flag.WifiMsgFlag, i,
										   LE, RE);
				HAL_USB_Log_Printf("\n[%d] task status:%d %d", ControlGetTick(), LE, RE);
				skip_loop = 1;
				break;
			}
			e->Rk3588_Flag.WifiMsgFlag = 0;
		}
		if (e->State_Bit.Exit)
		{
			Rk3588_Task_Cancel;
			break;
		}
	}
	Ctrl_Msg_Printf("Centering");
	HAL_Outage_Log(0, 0);
	// len = sprintf(msg_buffer, "End");
	// Message_Center_Send("PAGE1", 0, msg_buffer, len);
	/*HAL_Set_UI_Page1_Msg("End");*/
	uint8_t exitflag = e->State_Bit.Exit;
	e->State_Bit.IsRunning = 0;
	e->State_Bit.flag = 0;
	e->Error_Bit.flag = 0;
	if (exitflag == 0 &&
		e->Dev_Mode_Bit.LoopTest == 1 && skip_loop == 0)
	{
		SaftExitDelay(5000, 0);
		Message_Center_Send_prinft("Ctrl", 0,
								   0,
								   "ReqStrat");
	}
	goto BEGIN_POS;
}
static timetyped date(int y, int m, int d, int h, int min, int s)
{
    timetyped t = {0};
    t.date.year=y; t.date.month=m; t.date.date=d;
    t.date.hour=h; t.date.min=min; t.date.sec=s; return t;
}
static void configure(timetyped expiry, timetyped now)
{
    uint8_t marker[] = {0xaa, 0xaa};
    current_time = now;
    assert(HAL_CONFIG_WRITE(1, (uint8_t *)&expiry, sizeof(expiry)));
    assert(HAL_CONFIG_WRITE(3, marker, sizeof(marker)));
}
static void run_handler(void)
{
    idle_calls = movements = zero_calls = 0;
    if (!setjmp(handler_exit)) Task_control_handler(&control_info);
}
int main(void)
{
    uint8_t buf[50];
    timetyped end = date(2026,9,19,1,1,1);
    timetyped before = date(2026,9,19,1,1,0);
    memset(eeprom, 0xff, sizeof(eeprom));
    Verify_Init(); assert(Verify_Check_isExpired() == SeTboth);
    configure(end, before);
    int reads = read_calls;
    assert(Verify_Check_isExpired() == okay);
    assert(Task_control_Begin(&control_info));
    assert(read_calls == reads); /* Starting uses the cache, not EEPROM. */
    expire_pending = 1; pending_time = end;
    run_handler(); assert(!zero_calls && !movements && !control_info.State_Bit.isRun);
    assert(control_info.isExpired == expired);
    expire_pending = 0;
    int loads = task_loads;
    assert(!Task_control_Begin(&control_info)); assert(task_loads == loads);
    current_time.date.sec = 2; assert(Verify_Check_isExpired() == expired);
    current_time = before; assert(Verify_Check_isExpired() == okay);
    run_handler(); assert(zero_calls == 1);
    assert(Verify_Compare_time(&end, date(2027,1,1,0,0,0)) == expired);
    assert(Verify_Compare_time(&end, date(2025,12,31,23,59,59)) == okay);
    assert(Verify_Compare_time(&end, date(2026,10,1,0,0,0)) == expired);
    assert(Verify_Compare_time(&end, date(2026,9,18,23,59,59)) == okay);
    assert(Verify_Compare_time(&end, date(2026,9,19,0,59,59)) == okay);
    assert(Verify_Compare_time(&end, date(2026,9,19,1,0,59)) == okay);
    assert(Verify_Compare_time(&end, date(2026,9,19,1,2,0)) == expired);
    configure(InfiniteTag, before); assert(Verify_Check_isExpired() == okay);
    rtc_failed = 1; assert(Verify_Check_isExpired() == SetTimeOnly); rtc_failed = 0;
    current_time.date.month = 0; assert(Verify_Check_isExpired() == SetTimeOnly);
    current_time = before;
    timetyped bad = date(2026,2,29,1,0,0);
    HAL_CONFIG_WRITE(1, (uint8_t *)&bad, sizeof(bad)); assert(Verify_Check_isExpired() == NoTag);
    bad = date(2024,2,29,1,0,0); assert(Verify_Compare_time(&bad, date(2024,2,28,23,59,59)) == okay);
    HAL_CONFIG_WRITE(1, (uint8_t *)&end, 2); assert(Verify_Check_isExpired() == NoTag);
    configure(end, before);
    eeprom[32+2] ^= 1; assert(!HAL_CONFIG_READ(1,buf,sizeof(buf)));
    assert(Verify_Check_isExpired() == NoTag);
    configure(end, before);
    eeprom[32+1] = 255; assert(!HAL_CONFIG_READ(1,buf,sizeof(buf))); assert(Verify_Check_isExpired() == NoTag);
    configure(end, before); read_failed=1;
    assert(!HAL_CONFIG_READ(1,buf,sizeof(buf))); assert(Verify_Check_isExpired() == NoTag); read_failed=0;
    configure(end, before);
    assert(!HAL_CONFIG_READ(1,buf,1)); assert(Verify_Check_isExpired() == NoTag);
    configure(end, before);
    eeprom[96+2] ^= 1; assert(!HAL_CONFIG_READ(3,buf,sizeof(buf)));
    assert(Verify_Check_isExpired() == SeTboth);
    configure(end, end); write_failed=1;
    timetyped future = date(2027,1,1,0,0,0);
    assert(!HAL_CONFIG_WRITE(1,(uint8_t *)&future,sizeof(future)));
    assert(Verify_Check_isExpired() == expired); write_failed=0;
    assert(HAL_CONFIG_WRITE(1,(uint8_t *)&future,sizeof(future)));
    assert(Verify_Check_isExpired() == okay);
    write_failed=1;
    assert(!HAL_CONFIG_WRITE(1,buf,0));
    assert(Verify_Check_isExpired() == okay); /* Failed clear keeps actual ROM authorization. */
    write_failed=0;
    memset(&Verify_Cache,0,sizeof(Verify_Cache)); Verify_Init(); assert(Verify_Check_isExpired() == okay);
    uint8_t invalid_marker[] = {0,0}; HAL_CONFIG_WRITE(3,invalid_marker,2);
    assert(Verify_Check_isExpired() == SeTboth);
    assert(Verify_Set_Time(&before)); assert(Verify_Check_isExpired() == okay);
    current_time=date(2026,11,30,1,2,3); Verify_Set_Subscribe(1);
    assert(Verify_Cache.licenseDate.date.month == 12);
    current_time=date(2026,12,31,1,2,3); Verify_Set_Subscribe(2);
    assert(Verify_Cache.licenseDate.date.year == 2027);
    assert(Verify_Cache.licenseDate.date.month == 2 && Verify_Cache.licenseDate.date.date == 28);
    Verify_Factory_Reset(); assert(!Verify_Cache.licenseValid && !Verify_Cache.timeConfigured);
    assert(!Task_control_Begin(&control_info));
    config_mutex=NULL; mutex_failed=1;
    assert(!HAL_CONFIG_READ(1,buf,sizeof(buf)));
    assert(!Task_control_Begin(&control_info));
    puts("PASS: expiry boundaries, calendar transitions, permanent license, invalid ROM/RTC,");
    puts("      cache/readback, failed writes, renewal/reboot/reset, and both start gates.");
    return 0;
}
