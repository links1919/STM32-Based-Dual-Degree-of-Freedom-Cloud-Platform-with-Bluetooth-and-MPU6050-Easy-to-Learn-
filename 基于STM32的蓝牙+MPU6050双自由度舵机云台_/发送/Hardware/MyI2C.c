#include "stm32f10x.h"                  // Device header
#include "Delay.h"

/**
 * @brief 写入I2C时钟线SCL的电平
 * @param BitValue 要写入的电平值（0或1）
 * @retval 无
 */
void MyI2C_W_SCL(uint8_t BitValue){
	GPIO_WriteBit(GPIOB, GPIO_Pin_10, (BitAction)BitValue);  // 设置PB10引脚电平
	Delay_us(10);  // 延时10微秒，确保信号稳定
}

/**
 * @brief 写入I2C数据线SDA的电平
 * @param BitValue 要写入的电平值（0或1）
 * @retval 无
 */
void MyI2C_W_SDA(uint8_t BitValue){
	GPIO_WriteBit(GPIOB, GPIO_Pin_11, (BitAction)BitValue);  // 设置PB11引脚电平
	Delay_us(10);  // 延时10微秒，确保信号稳定
}

/**
 * @brief 读取I2C数据线SDA的电平
 * @param 无
 * @retval 读取到的电平值（0或1）
 */
uint8_t MyI2C_R_SDA(void){
	uint8_t BitValue;
	BitValue = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11);  // 读取PB11引脚电平
	Delay_us(10);  // 延时10微秒，确保信号稳定
	return BitValue;  // 返回读取到的电平值
}

/**
 * @brief I2C总线初始化函数
 * @param 无
 * @retval 无
 */
void MyI2C_Init(void){
	// 使能GPIOB时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	// 配置GPIO引脚
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;  // 开漏输出模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;  // PB10(SCL)和PB11(SDA)
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  // 引脚速度50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);  // 初始化GPIOB
	
	// 将SCL和SDA初始化为高电平
	GPIO_SetBits(GPIOB, GPIO_Pin_10 | GPIO_Pin_11);
}

/**
 * @brief 发送I2C开始信号
 * @param 无
 * @retval 无
 */
void MyI2C_Start(void){
	MyI2C_W_SDA(1);  // 确保SDA为高电平
	MyI2C_W_SCL(1);  // 确保SCL为高电平
	MyI2C_W_SDA(0);  // 在SCL为高电平时，拉低SDA产生开始信号
	MyI2C_W_SCL(0);  // 拉低SCL，准备发送数据
}

/**
 * @brief 发送I2C停止信号
 * @param 无
 * @retval 无
 */
void MyI2C_Stop(void){
	MyI2C_W_SDA(0);  // 确保SDA为低电平
	MyI2C_W_SCL(1);  // 拉高SCL
	MyI2C_W_SDA(1);  // 在SCL为高电平时，拉高SDA产生停止信号
}

/**
 * @brief 发送一个字节的数据
 * @param Byte 要发送的字节数据
 * @retval 无
 */
void MyI2C_SendByte(uint8_t Byte){
	uint8_t i;
	for(i=0; i<8; i++){  // 循环8次，发送8位数据
		MyI2C_W_SDA(Byte & (0x80 >> i));  // 发送当前位（从最高位开始）
		MyI2C_W_SCL(1);  // 拉高SCL，让从机采样
		MyI2C_W_SCL(0);  // 拉低SCL，准备发送下一位
	}
}

/**
 * @brief 接收一个字节的数据
 * @param 无
 * @retval 接收到的字节数据
 */
uint8_t MyI2C_ReceiveByte(void){
	uint8_t i, Byte=0x00;
	MyI2C_W_SDA(1);  // 释放SDA，让从机发送数据
	for(i=0; i<8; i++){  // 循环8次，接收8位数据
		MyI2C_W_SCL(1);  // 拉高SCL，读取数据
		if(MyI2C_R_SDA() == 1){  // 如果读取到高电平
			Byte |= (0x80 >> i);  // 设置当前位为1
		}
		MyI2C_W_SCL(0);  // 拉低SCL，准备接收下一位
	}
	return Byte;  // 返回接收到的字节数据
}

/**
 * @brief 发送应答信号
 * @param AckBit 应答位值（0表示应答，1表示非应答）
 * @retval 无
 */
void MyI2C_SendAck(uint8_t AckBit){
	MyI2C_W_SDA(AckBit);  // 设置应答位
	MyI2C_W_SCL(1);  // 拉高SCL，让从机采样
	MyI2C_W_SCL(0);  // 拉低SCL
}

/**
 * @brief 接收应答信号
 * @param 无
 * @retval 接收到的应答位值（0表示应答，1表示非应答）
 */
uint8_t MyI2C_ReceiveAck(void){
	uint8_t AckBit;
	MyI2C_W_SDA(1);  // 释放SDA
	MyI2C_W_SCL(1);  // 拉高SCL，读取应答位
	AckBit = MyI2C_R_SDA();  // 读取应答位
	MyI2C_W_SCL(0);  // 拉低SCL
	return AckBit;  // 返回应答位值
}