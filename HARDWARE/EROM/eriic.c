#include "eriic.h"
#include "delay.h"	 
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//电容触摸屏-IIC 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/7
//版本：V1.1
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
//********************************************************************************
//修改说明
//V1.1 20140721
//1,修改ER_IIC_Read_Byte函数,读数据更快.
//2,修改ER_IIC_Wait_Ack函数,以支持MDK的-O2优化.
////////////////////////////////////////////////////////////////////////////////// 	
//控制I2C速度的延时
void ER_Delay(void)
{
	delay_us(2);
} 
//电容触摸芯片IIC接口初始化
void ER_IIC_Init(void)
{            
	gpio_init_type gpio_init_struct;
	crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
	crm_periph_clock_enable(CRM_GPIOF_PERIPH_CLOCK, TRUE);

	gpio_default_para_init(&gpio_init_struct);
	// PB0 as output pull-up (SCL)
	gpio_init_struct.gpio_pins = GPIO_PINS_7;
	gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
	gpio_init_struct.gpio_out_type = GPIO_OUTPUT_OPEN_DRAIN;
	gpio_init_struct.gpio_pull = GPIO_PULL_UP;
	gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
	gpio_init(GPIOG, &gpio_init_struct);

	// PF11 as output pull-up (SDA)
	gpio_init_struct.gpio_pins = GPIO_PINS_8;
	gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
	gpio_init_struct.gpio_out_type = GPIO_OUTPUT_OPEN_DRAIN;
	gpio_init_struct.gpio_pull = GPIO_PULL_UP;
	gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
	gpio_init(GPIOG, &gpio_init_struct);
}
//产生IIC起始信号
void ER_IIC_Start(void)
{
	ER_IIC_SDA=1;	  	  
	ER_IIC_SCL=1;
	delay_us(30);
 	ER_IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	ER_Delay();
	ER_IIC_SCL=0;//钳住I2C总线，准备发送或接收数据 
}	  
//产生IIC停止信号
void ER_IIC_Stop(void)
{
	ER_IIC_SCL=1;
	delay_us(30);
	ER_IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
	ER_Delay();
	ER_IIC_SDA=1;//发送I2C总线结束信号  
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 ER_IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	ER_IIC_SDA=1;	   
	ER_IIC_SCL=1;
	ER_Delay();
	while(ER_READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			ER_IIC_Stop();
			return 1;
		} 
		ER_Delay();
	}
	ER_IIC_SCL=0;//时钟输出0 	   
	return 0;  
} 
//产生ACK应答
void ER_IIC_Ack(void)
{
	ER_IIC_SCL=0;

	ER_Delay();
	ER_IIC_SDA=0;
	ER_Delay();
	ER_IIC_SCL=1;
	ER_Delay();
	ER_IIC_SCL=0;
}
//不产生ACK应答		    
void ER_IIC_NAck(void)
{
	ER_IIC_SCL=0;
	ER_Delay();
	ER_IIC_SDA=1;
	ER_Delay();
	ER_IIC_SCL=1;
	ER_Delay();
	ER_IIC_SCL=0;
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void ER_IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
    ER_IIC_SCL=0;//拉低时钟开始数据传输
	ER_Delay();
	for(t=0;t<8;t++)
    {              
        ER_IIC_SDA=(txd&0x80)>>7;
        txd<<=1; 	      
		ER_IIC_SCL=1; 
		ER_Delay();
		ER_IIC_SCL=0;	
		ER_Delay();
    }	 
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 ER_IIC_Read_Byte(unsigned char ack)
{
	u8 i,receive=0;
	delay_us(30);
	for(i=0;i<8;i++ )
	{ 
		ER_IIC_SCL=0; 	    	   
		ER_Delay();
		ER_IIC_SCL=1;
        ER_Delay();	 
		receive<<=1;
		if(ER_READ_SDA)receive++;   
	}	  				 
	if (!ack)ER_IIC_NAck();//发送nACK
	else ER_IIC_Ack(); //发送ACK   
 	return receive;
}





























