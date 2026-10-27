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
