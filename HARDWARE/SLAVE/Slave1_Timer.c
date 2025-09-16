#include "Slave1_Timer.h"
#include "Slave1_IO.h"
#include "at32f435_437.h"
#include "math.h"

Slave_Function_CB function_cb;
void TIM1_BRK_TIM9_IRQHandler(void)
{

    if (TMR9->ists_bit.ovfif) // ????????????
    {
        if (function_cb != 0)
        {
            uint8_t res = function_cb();
            if (res)
            {
                TMR9->ctrl1_bit.tmren = 0;
                extern void Slave1_En_IO(uint8_t state);
                Slave1_En_IO(0);
            }
        }
        // 补停机代码
    }
    TMR9->ists = 0;
}
// 当handler返回1时停止状态机
void Slave1_Set_Machine_Cb(Slave_Function_CB cb)
{
    Slave1_IO_Init();
    Slave1_En_IO(1);
    // Slave1_Step_Generator_Init(50000, 10);
    Slave_server_Init();
    TMR9->ctrl1_bit.tmren = 0;
    function_cb = cb;
    TMR9->ctrl1_bit.tmren = 1;
}

void Step_Phase_Set(u32 arr, u32 psc)
{
    if (arr == 0 && psc == 0)
        TMR1->ctrl1_bit.tmren = 0;
    TMR1->pr = psc;
    TMR1->cval = arr;
    TMR1->ctrl1_bit.tmren = 1;
}

void SRV1_Encoder_Init()
{
    // const uint32_t ARR_VALUE = 2097156;
    const uint16_t ch1_pin = 5;
    const uint16_t ch2_pin = 3;
    CRM->apb1en_bit.tmr2

    GPIOA->MODER |= (0x02 << ch1_pin * 2);
    GPIOB->MODER |= (0x02 << ch2_pin * 2);

    GPIOA->AFR[0] |= 0x01 << 20; // set tmr 2 alternate channel
    GPIOB->AFR[0] |= 0x01 << 12;

    TIM2->ARR = 0xffffffff;
    TIM2->PSC = 0;
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
uint8_t Slave_Back(int Tag_Pos)
{
    return 1;
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

float Slave1_Get_Encode_Angle()
{
    int32_t count = TIM2->CNT;
    float angle = count / 67108800.0f * 360 * 8;
    return angle;
}
// void Slave1_CMD(Slave1_CMD_Typed cmd,int freq)
// {

// }
