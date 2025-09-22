#include "Slave1_Timer.h"
#include "Slave1_IO.h"
#include "at32f435_437.h"
#include "math.h"

float acc_angle = 0;
float soft_angle_CNT;
Slave_Function_CB function_cb;
void TIM1_BRK_TIM9_IRQHandler(void)
{
    if (TIM9->SR & TIM_IT_Update) // ????????????
    {
        if (function_cb != 0)
        {
            uint8_t res = function_cb();
            if (res)
            {
                function_cb = 0;
                extern void Slave1_En_IO(uint8_t state);
                Slave1_En_IO(0);
                extern void Motor_Set_Speed(float speed);
                extern void Motor_Spd_Pid();
                Motor_Set_Speed(0);
                Motor_Spd_Pid(); // avoid c601 unexpect movement
            }
            Motor_Spd_Pid(); // avoid c601 unexpect movement
        }
        else
        {
            //							Motor_Set_Speed(0);
        }
        acc_angle = Slave1_Get_Encode_Angle();
        //        extern void Motor_Spd_Pid();
        //        Motor_Spd_Pid(); // avoid c601 unexpect movement
        // 补停机代码
    }
    TIM9->SR = (uint16_t)~TIM_IT_Update;
}
// 当handler返回1时停止状态机
void Slave1_Set_Machine_Cb(Slave_Function_CB cb)
{
    Slave1_IO_Init();
    //    Slave1_En_IO(1);
    // Slave1_Step_Generator_Init(50000, 10);
    Slave_server_Init();
    TIM9->CR1 &= ~TIM_CR1_CEN;
    function_cb = cb;
    TIM9->CR1 |= TIM_CR1_CEN;
}

uint8_t Slave1_Get_Machine_state()
{
    if (function_cb != 0)
        return 1;
    else
        return 0;
}

void Step_Phase_Set(u32 arr, u32 psc)
{
    if (arr == 0 && psc == 0)
    {
        TIM_Cmd(TIM1, DISABLE);
        return;
    }
    Slave1_En_IO(1);
#if 1
    // 此部分需手动修改IO口设置
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    // TIM_OCInitTypeDef  TIM_OCInitStructure;

    TIM_TimeBaseStructure.TIM_Prescaler = psc;                  // 定时器分频
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数模式
    TIM_TimeBaseStructure.TIM_Period = arr;                     // 自动重装载值
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;

    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); // 初始化定时器14
    TIM1->CCR1 = arr / 2;

    TIM_Cmd(TIM1, ENABLE); // 使能TIM14
#else
    TIM1->CR1 &= ~TIM_CR1_CEN;
    uint16_t cval = TIM1->CNT + 1;
    TIM1->PSC = 0;
    TIM1->PSC = psc;
    if (TIM1->CCR1 > TIM1->CNT)
        TIM1->CCR1 = 0xffff;
    else
        TIM1->CCR1 = 0x0;
    deg = (float)TIM1->CNT / TIM1->ARR;
    TIM1->CNT = (arr)*cval / (TIM1->ARR + 1) - 1;
    TIM1->ARR = arr - 1;
    TIM1->CCR1 = arr / 2 - 1;
    TIM1->CR1 |= TIM_CR1_CEN;
#endif
}

void SRV1_Encoder_Init()
{
    static uint8_t isinit = 0;
    if (isinit != 0)
        return;
    isinit = 1;
    // const uint32_t ARR_VALUE = 2097156;
    const uint16_t ch1_pin = 5;
    const uint16_t ch2_pin = 3;
    RCC->APB1ENR |= (RCC_APB1ENR_TIM2EN);
    RCC->AHB2ENR |= RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOAEN;
    // Configure TIM2 in Encoder mode 3 (count on both TI1FP1 and TI2FP2 edges)
    TIM2->SMCR &= ~TIM_SMCR_SMS;
    TIM2->SMCR |= TIM_SMCR_SMS_0 | TIM_SMCR_SMS_1; // Encoder mode 3

    // Configure input capture for both channels
    TIM2->CCMR1 &= ~(TIM_CCMR1_CC1S | TIM_CCMR1_CC2S);
    TIM2->CCMR1 |= TIM_CCMR1_CC1S_1 | TIM_CCMR1_CC2S_1; // CC1 channel as input, mapped on TI1; CC2 channel as input, mapped on TI2

    // Set the polarity and filter (if needed)
    TIM2->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P);    // Non-inverted polarity
    TIM2->CCMR1 &= ~(TIM_CCMR1_IC1F | TIM_CCMR1_IC2F); // No filter

    // Enable the counter
    TIM2->CR1 |= TIM_CR1_CEN;
    // TIM2->SMCR |= 0x03; // use clock input encoder c mode

    // TIM2->CCMR1 |= 0x01;
    // TIM2->CCMR1 |= (0x01 << 8);

    // TIM2->CCER |= TIM_CCER_CC1E;
    // TIM2->CCER |= TIM_CCER_CC2E;

    GPIOA->MODER |= (0x02 << ch1_pin * 2);
    GPIOB->MODER |= (0x02 << ch2_pin * 2);

    GPIOA->AFR[0] |= 0x01 << 20; // set tmr 2 alternate channel
    GPIOB->AFR[0] |= 0x01 << 12;

    TIM2->ARR = 1073741;
    TIM2->PSC = 0;
    TIM2->CR1 |= TIM_CR1_ARPE;
    TIM2->CR1 |= TIM_CR1_CEN;
}

void Slave_server_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE); /// 使能TIM9时钟

    TIM_TimeBaseInitStructure.TIM_Period = 499; // 自动重装载值
#ifdef HARDWARE_TEST
    TIM_TimeBaseInitStructure.TIM_Prescaler = 167; // 定时器分频
#else
    TIM_TimeBaseInitStructure.TIM_Prescaler = 336 - 1; // 定时器分频
#endif
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数模式
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;

    TIM_TimeBaseInit(TIM9, &TIM_TimeBaseInitStructure); // 初始化TIM9

    TIM_ITConfig(TIM9, TIM_IT_Update, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = TIM1_BRK_TIM9_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    SRV1_Encoder_Init();
}

void Slave1_Step_Generator_Init(uint32_t arr, uint32_t psc)
{
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    TIM1->CCR1 = arr / 2;

    GPIOA->MODER &= ~GPIO_MODER_MODER8;
    GPIOA->MODER |= GPIO_MODER_MODER8_1;

    GPIOA->AFR[1] |= 0x1 << (4 * (8 - 8)); // PA8 corresponds to AFR[1] (AFRH), 0th position

    // Set the prescaler value to have TIM1 counter clock = 1 MHz
    TIM1->PSC = 84 - 1; // APB2 clock is 84 MHz

    // Set the ARR value to have a PWM frequency of 1 kHz
    TIM1->ARR = 1000 - 1;

    // Set the CCR1 value to have a duty cycle of 50%
    TIM1->CCR1 = 500;

    // Configure the TIM1 channel 1 in PWM mode 1
    TIM1->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM1->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;

    // Enable the output compare 1 preload register
    TIM1->CCMR1 |= TIM_CCMR1_OC1PE;

    // Enable the capture/compare 1 output
    TIM1->CCER |= TIM_CCER_CC1E;

    // Enable auto-reload preload
    TIM1->CR1 |= TIM_CR1_ARPE;

    // Enable the TIM1 main output
    TIM1->BDTR |= TIM_BDTR_MOE;

    // Enable the TIM1 counter
    TIM1->CR1 |= TIM_CR1_CEN;
    TIM1->CCR1 = arr / 2;

    Slave_server_Init();
}

void tim_f_set(int f)
{

    TIM_TypeDef *TIMx = TIM1;
    u8 Mins_flag;
    int time_arr, time_psc;
    int a, clk = 84000000, c, y, r = 0, t, i;
    if (f == 0)
    {
        TIM_Cmd(TIM1, 0);
        return;
    }
    if (f < 0)
    {
        f = -f;
        Mins_flag = 0;
    }
    else
        Mins_flag = 1;
    a = clk / (f);
    c = sqrt(a);
    y = 10000000;
    for (i = 2; i <= c; i++)
    {
        t = a % (i);

        if (y > t && a / i < 65536)
        {

            y = t;

            r = i;
            if (y == 0 && a / i < 65536)
                break;
        }
    }
    c = 10;

    time_arr = a / r - 1;
    time_psc = r - 1;
    // Out_Put_data = sys_clk / time_arr / time_psc;
    if (f > 1000)
    {
        Slave1_Dir_IO(Mins_flag);
        if (time_arr != TIMx->ARR || time_psc != TIMx->PSC)
            Step_Phase_Set(time_arr, time_psc);
    }
    TIMx->CR1 |= TIM_CR1_CEN;
}

void tim_f_sin_set(int f)
{
    TIM_TypeDef *TIMx = TIM1;
    float angle = -f;
    soft_angle_CNT += (angle / angle_step) / 1000;
    if (soft_angle_CNT > 180)
    {
        soft_angle_CNT -= 360;
    }
    else if (soft_angle_CNT < -180)
    {
        soft_angle_CNT += 360;
    }
    u8 Mins_flag;
    int time_arr, time_psc;
    int a, clk = 84000000, c, y, r = 0, t, i;
    if (f == 0)
    {
        Step_Phase_Set(0, 0);
        return;
    }
    if (f < 0)
    {
        f = -f;
        Mins_flag = 0;
    }
    else
        Mins_flag = 1;
    // if (f < 10)
    //     return;
    a = clk / (f);
    c = sqrt(a);
    y = 10000000;
    for (i = 2; i <= c; i++)
    {
        t = a % (i);

        if (y > t && a / i < 65536)
        {

            y = t;

            r = i;
            if (y == 0 && a / i < 65536)
                break;
        }
    }
    c = 10;

    time_arr = a / r - 1;
    time_psc = r - 1;
    if (f > 1000 || TIMx->CNT > TIMx->CCR3)
    {
        Slave1_Dir_IO(Mins_flag);
        if (time_arr != TIMx->ARR || time_psc != TIMx->PSC)
            Step_Phase_Set(time_arr, time_psc);
    }
    TIMx->CR1 |= TIM_CR1_CEN;
}

void tim_f_sin_set_Dir(int f)
{
    TIM_TypeDef *TIMx = TIM1;
    u8 Mins_flag;
    int time_arr, time_psc;
    int a, clk = 84000000, c, y, r = 0, t, i;
    if (f == 0)
    {
        Step_Phase_Set(0, 0);
        return;
    }
    if (f < 0)
    {
        f = -f;
        Mins_flag = 0;
    }
    else
        Mins_flag = 1;
    // if (f < 10)
    //     return;
    a = clk / (f);
    c = sqrt(a);
    y = 10000000;
    for (i = 2; i <= c; i++)
    {
        t = a % (i);

        if (y > t && a / i < 65536)
        {

            y = t;

            r = i;
            if (y == 0 && a / i < 65536)
                break;
        }
    }
    c = 10;

    time_arr = a / r - 1;
    time_psc = r - 1;
    Slave1_Dir_IO(Mins_flag);
    if (time_arr != TIMx->ARR || time_psc != TIMx->PSC)
        Step_Phase_Set(time_arr, time_psc);
    TIMx->CR1 |= TIM_CR1_CEN;
}

uint8_t Slave_Back(int Tag_Pos)
{
    return 1;
}

void Slave_Release()
{
    Slave1_En_IO(0);
    TIM_Cmd(TIM1, 0);
}

u8 Slave_Back_spd(int Tag_Pos, int spd)
{
    return 1;
}
u8 slave_sin_back(void)
{
    return 1;
}

int32_t Slave1_Get_Encode()
{
    return (int32_t)TIM2->CNT;
}

void Slave1_Set_Encode(uint32_t cnt)
{
    TIM2->CNT = cnt;
}

int32_t count_record;
int8_t isEncode_error = 0;
uint8_t Slave1_Check_Encode_Angle()
{
    static uint8_t flag = 0;
    int32_t count = *(int32_t *)&(TIM2->CNT);
    
    if (flag == 0)
    {
        count_record = count;
        flag = 1;
        return 0;
    }
    if (flag == 1)
    {
        int absvalue = abs(count_record - count);
        if (absvalue < 10)
        {
            isEncode_error = 1;
            return 1;
        }
        return 0;
    }
    return 0;
}

float Slave1_Get_Encode_Angle()
{
//    if (isEncode_error)
    {
        return soft_angle_CNT;
    }
    int32_t count = *(int32_t *)&(TIM2->CNT);
    count = count - 1073741 / 2;
    float angle = count / 1073741.0f * 360.0f;
    return angle;
}

float Slave1_Get_Encode_Angle_real()
{

    int32_t count = *(int32_t *)&(TIM2->CNT);
    count = count - 1073741 / 2;
    float angle = count / 1073741.0f * 360.0f;
    return angle;
}

uint8_t Slave1_get_Encode_error()
{
    return isEncode_error;
}

void Slave1_Set_Encode_Angle(float angle)
{
    	soft_angle_CNT = angle;
    angle = angle + 180;
	
    int32_t *count = (int32_t *)&(TIM2->CNT);
    *count = (angle / 360) * 1073741.0f;
}
// void Slave1_CMD(Slave1_CMD_Typed cmd,int freq)
// {

// }
