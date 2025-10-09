#include "eriic.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////
// 本程序只供学习使用，未经作者许可，不得用于其它任何用途
// ALIENTEK STM32F407开发板
// 电容触摸屏-IIC 驱动代码
// 正点原子@ALIENTEK
// 技术论坛:www.openedv.com
// 创建日期:2014/5/7
// 版本：V1.1
// 版权所有，盗版必究。
// Copyright(C) 广州市星翼电子科技有限公司 2014-2024
// All rights reserved
//********************************************************************************
// 修改说明
// V1.1 20140721
// 1,修改ER_IIC_Read_Byte函数,读数据更快.
// 2,修改ER_IIC_Wait_Ack函数,以支持MDK的-O2优化.
//////////////////////////////////////////////////////////////////////////////////
// 控制I2C速度的延时
void ER_Delay(void)
{
	delay_us(5);
}
// 电容触摸芯片IIC接口初始化
void ER_IIC_Init(void)
{
	gpio_init_type gpio_init_struct;
	crm_periph_clock_enable(CRM_GPIOG_PERIPH_CLOCK, TRUE);

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
// 产生IIC起始信号
void ER_IIC_Start(void)
{
	ER_IIC_SDA = 1;
	ER_IIC_SCL = 1;
	delay_us(5);
	ER_IIC_SDA = 0; // START:when CLK is high,DATA change form high to low
	ER_Delay();
	ER_IIC_SCL = 0; // 钳住I2C总线，准备发送或接收数据
}
// 产生IIC停止信号
void ER_IIC_Stop(void)
{
	ER_IIC_SCL = 1;
	delay_us(5);
	ER_IIC_SDA = 0; // STOP:when CLK is high DATA change form low to high
	ER_Delay();
	ER_IIC_SDA = 1; // 发送I2C总线结束信号
}
// 等待应答信号到来
// 返回值：1，接收应答失败
//         0，接收应答成功
u8 ER_IIC_Wait_Ack(void)
{
	u8 ucErrTime = 0;
	ER_IIC_SDA = 1;
	ER_Delay();
	ER_IIC_SCL = 1;
	ER_Delay();
	while (ER_READ_SDA)
	{
		ucErrTime++;
		if (ucErrTime > 250)
		{
			ER_IIC_Stop();
			return 1;
		}
		ER_Delay();
	}
	ER_IIC_SCL = 0; // 时钟输出0
	return 0;
}
// 产生ACK应答
void ER_IIC_Ack(void)
{
	ER_IIC_SCL = 0;

	ER_Delay();
	ER_IIC_SDA = 0;
	ER_Delay();
	ER_IIC_SCL = 1;
	ER_Delay();
	ER_IIC_SCL = 0;
}
// 不产生ACK应答
void ER_IIC_NAck(void)
{
	ER_IIC_SCL = 0;
	ER_Delay();
	ER_IIC_SDA = 1;
	ER_Delay();
	ER_IIC_SCL = 1;
	ER_Delay();
	ER_IIC_SCL = 0;
}
// IIC发送一个字节
// 返回从机有无应答
// 1，有应答
// 0，无应答
void ER_ER_IIC_Send_Byte(u8 txd)
{
	u8 t;
	ER_IIC_SCL = 0; // 拉低时钟开始数据传输
	ER_Delay();
	for (t = 0; t < 8; t++)
	{
		ER_IIC_SDA = (txd & 0x80) >> 7;
		txd <<= 1;
		ER_IIC_SCL = 1;
		ER_Delay();
		ER_IIC_SCL = 0;
		ER_Delay();
	}
	ER_IIC_SDA = 1;
}
// 读1个字节，ack=1时，发送ACK，ack=0，发送nACK
u8 ER_IIC_Read_one_Bytes(unsigned char ack)
{
	u8 i, receive = 0;
	delay_us(5);
	for (i = 0; i < 8; i++)
	{
		ER_IIC_SCL = 0;
		ER_Delay();
		ER_IIC_SCL = 1;
		ER_Delay();
		receive <<= 1;
		if (ER_READ_SDA)
			receive++;
	}
	ER_IIC_SCL=0;
	if (!ack)
		ER_IIC_NAck(); // 发送nACK
	else
		ER_IIC_Ack(); // 发送ACK
	return receive;
}

uint8_t ER_IIC_Read_Bytes(uint8_t I2C_Addr, uint16_t addr, uint8_t *buf, uint8_t len)
{
	uint8_t i;
	ER_IIC_Start();
	ER_ER_IIC_Send_Byte(I2C_Addr); // 发送器件地址+写信号
	ER_IIC_Wait_Ack();
	ER_ER_IIC_Send_Byte(addr >> 8); // 发送寄存器地址
	ER_IIC_Wait_Ack();
	ER_ER_IIC_Send_Byte(addr & 0xFF); // 发送寄存器地址
	ER_IIC_Wait_Ack();
	ER_IIC_Start();
	ER_ER_IIC_Send_Byte(I2C_Addr + 1); // 发送器件地址+读信号
	ER_IIC_Wait_Ack();
	for (i = 0; i < len; i++)
	{
		if (i != len - 1)
			*buf++ = ER_IIC_Read_one_Bytes(1); // 带ACK
		else
			*buf++ = ER_IIC_Read_one_Bytes(0); // 最后一个字节NACK
	}
	ER_IIC_Stop(); // 产生一个停止条件
	return 0;
}

//void atk_24cxx_init(void)
//{
//	iic_init();
//}

/**
 * @brief       在ATK_24CXX指定地址读出一个数据
 * @param       readaddr: 开始读数的地址
 * @retval      读到的数据
 */
uint8_t atk_24cxx_read_one_byte(uint16_t addr)
{
	uint8_t temp = 0;
	ER_IIC_Start(); /* 发送起始信号 */

	ER_ER_IIC_Send_Byte(0XA0);	  /* 发送写命令, IIC规定最低位是0, 表示写入 */
	ER_IIC_Wait_Ack();			  /* 每次发送完一个字节,都要等待ACK */
	ER_ER_IIC_Send_Byte(addr >> 8); /* 发送高字节地址 */

	ER_IIC_Wait_Ack();			   /* 每次发送完一个字节,都要等待ACK */
	ER_IIC_Send_Byte(addr % 256); /* 发送低位地址 */
	ER_IIC_Wait_Ack();			   /* 等待ACK, 此时地址发送完成了 */

	iic_start();			 /* 重新发送起始信号 */
	ER_IIC_Send_Byte(0XA1);	 /* 进入接收模式, IIC规定最低位是0, 表示读取 */
	ER_IIC_Wait_Ack();			 /* 每次发送完一个字节,都要等待ACK */
	temp = ER_IIC_Read_one_Bytes(0); /* 接收一个字节数据 */
	iic_stop();				 /* 产生一个停止条件 */
	return temp;
}

/**
 * @brief       在ATK_24CXX指定地址写入一个数据
 * @param       addr: 写入数据的目的地址
 * @param       data: 要写入的数据
 * @retval      无
 */
void atk_24cxx_write_one_byte(uint16_t addr, uint8_t data)
{
	/* 原理说明见:at24cxx_read_one_byte函数, 本函数完全类似 */
	iic_start(); /* 发送起始信号 */
	{
		ER_IIC_Send_Byte(0XA0);	  /* 发送写命令, IIC规定最低位是0, 表示写入 */
		ER_IIC_Wait_Ack();			  /* 每次发送完一个字节,都要等待ACK */
		ER_IIC_Send_Byte(addr >> 8); /* 发送高字节地址 */
	}

	ER_IIC_Wait_Ack();			   /* 每次发送完一个字节,都要等待ACK */
	ER_IIC_Send_Byte(addr % 256); /* 发送低位地址 */
	ER_IIC_Wait_Ack();			   /* 等待ACK, 此时地址发送完成了 */

	/* 因为写数据的时候,不需要进入接收模式了,所以这里不用重新发送起始信号了 */
	ER_IIC_Send_Byte(data); /* 发送1字节 */
	ER_IIC_Wait_Ack();		 /* 等待ACK */
	iic_stop();			 /* 产生一个停止条件 */
	delay_ms(10);		 /* 注意: EEPROM 写入比较慢,必须等到10ms后再写下一个字节 */
}

/**
 * @brief       在ATK_24CXX里面的指定地址开始读出指定个数的数据
 * @param       addr    : 开始读出的地址 对24c02为0~255
 * @param       pbuf    : 数据数组首地址
 * @param       datalen : 要读出数据的个数
 * @retval      无
 */
void atk_24cxx_read(uint16_t addr, uint8_t *pbuf, uint16_t datalen)
{
	while (datalen--)
	{
		*pbuf++ = atk_24cxx_read_one_byte(addr++);
	}
}

/**
 * @brief       在ATK_24CXX里面的指定地址开始写入指定个数的数据
 * @param       addr    : 开始写入的地址 对24c02为0~255
 * @param       pbuf    : 数据数组首地址
 * @param       datalen : 要写入数据的个数
 * @retval      无
 */
void atk_24cxx_write(uint16_t addr, uint8_t *pbuf, uint16_t datalen)
{
	while (datalen--)
	{
		atk_24cxx_write_one_byte(addr, *pbuf);
		addr++;
		pbuf++;
	}
}

uint8_t ER_IIC_Write_Bytes(uint8_t I2C_Addr, uint16_t addr, uint8_t *buf, uint8_t len)
{
	uint8_t i;
	ER_IIC_Start();
	ER_ER_IIC_Send_Byte(I2C_Addr); // 发送器件地址+写信号
	if (ER_IIC_Wait_Ack())
	{
		ER_IIC_Stop();
		return 1;
	}
	ER_ER_IIC_Send_Byte(addr >> 8); // 发送寄存器地址
	ER_IIC_Wait_Ack();
	ER_ER_IIC_Send_Byte(addr & 0xFF); // 发送寄存器地址
	ER_IIC_Wait_Ack();
	for (i = 0; i < len; i++)
	{
		ER_ER_IIC_Send_Byte(*buf++); // 发送数据
		if (ER_IIC_Wait_Ack())
		{
			ER_IIC_Stop();
			return 1;
		}
	}
	ER_IIC_Stop(); // 产生一个停止条件
	return 0;
}

uint8_t M24C64_Write_Byte(uint8_t addr, uint8_t *buf, uint16_t len)
{
	const uint8_t m24_addr = 0xA0;
	uint16_t count = 0, remainCount = 0, i;
	uint8_t bl = 0;
	uint8_t *temp = buf;
	count = len >> 8;
	remainCount = len & 0xFF;
	for (i = 0; i < count + 1; i++)
	{
		if (i == count)
			bl = remainCount;
		else
			bl = 256;
		if (ER_IIC_Write_Bytes(m24_addr, addr + (i << 8), temp, bl))
			return 1;
		temp += bl;
	}
	return 0;
}

uint8_t M24C64_Read_Byte(uint8_t addr, uint8_t *buf, uint16_t len)
{
	const uint8_t m24_addr = 0xA0;
	uint16_t count = 0, remainCount = 0, i;
	uint8_t bl = 0;
	uint8_t *temp = buf;
	count = len >> 8;
	remainCount = len & 0xFF;
	for (i = 0; i < count + 1; i++)
	{
		if (i == count)
			bl = remainCount;
		else
			bl = 256;
		if (ER_IIC_Read_Bytes(m24_addr, addr + (i << 8), temp, bl))
			return 1;
		temp += bl;
	}
	return 0;
}
