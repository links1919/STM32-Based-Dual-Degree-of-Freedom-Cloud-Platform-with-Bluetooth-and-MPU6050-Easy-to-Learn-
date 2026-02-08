#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <stdarg.h>

/**
 * @brief 串口全局变量定义
 */
uint8_t Serial_TxPacket[4];  // 串口发送数据包
uint8_t Serial_RxPacket[4];  // 串口接收数据包
uint8_t Serial_RxFlag;       // 串口接收完成标志

/**
 * @brief 串口初始化函数
 * @param 无
 * @retval 无
 * @note 配置USART1，波特率9600，8位数据位，1位停止位，无校验位
 */
void Serial_Init(void){
	// 使能USART1和GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	// 配置PA9为USART1_TX引脚（复用推挽输出）
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// 配置PA10为USART1_RX引脚（上拉输入）
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// 配置USART1参数
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 9600;                     // 波特率9600
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  // 无硬件流控
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;  // 同时开启发送和接收
	USART_InitStructure.USART_Parity = USART_Parity_No;            // 无校验位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;         // 1位停止位
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;    // 8位数据位
	USART_Init(USART1, &USART_InitStructure);                      // 初始化USART1
	
	// 开启USART1接收中断
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
	// 配置NVIC优先级分组
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	// 配置USART1中断
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;              // USART1中断通道
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                // 开启中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;      // 抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;             // 子优先级1
	NVIC_Init(&NVIC_InitStructure);                                // 初始化NVIC
	
	// 使能USART1
	USART_Cmd(USART1, ENABLE);
}

/**
 * @brief 发送一个字节数据
 * @param Byte 要发送的字节数据
 * @retval 无
 */
void Serial_SendByte(uint8_t Byte){
	USART_SendData(USART1, Byte);  // 发送数据
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);  // 等待发送完成
}

/**
 * @brief 发送一个数组数据
 * @param Array 要发送的数组指针
 * @param Length 数组长度
 * @retval 无
 */
void Serial_SendArray(uint8_t *Array, uint16_t Length){
	uint16_t i;
	for(i=0; i<Length; i++){  // 循环发送数组中的每个字节
		Serial_SendByte(Array[i]);
	}
}

/**
 * @brief 发送一个字符串
 * @param String 要发送的字符串指针
 * @retval 无
 */
void Serial_SendString(char *String){
	uint8_t i;
	for(i=0; String[i] != '\0'; i++){  // 循环发送字符串中的每个字符，直到遇到结束符
		Serial_SendByte(String[i]);
	}
}

/**
 * @brief 计算X的Y次方
 * @param X 底数
 * @param Y 指数
 * @retval 计算结果
 */
uint32_t Serial_Pow(uint32_t X, uint32_t Y){
	uint32_t Result = 1;
	while(Y--){
		Result *= X;  // 循环相乘Y次
	}
	return Result;
}

/**
 * @brief 发送一个数字
 * @param Number 要发送的数字
 * @param Length 数字的位数
 * @retval 无
 */
void Serial_SendNumber(uint32_t Number, uint8_t Length){
	uint8_t i;
	for(i=0; i<Length; i++){  // 循环发送每一位数字
		Serial_SendByte(Number / Serial_Pow(10, Length - i - 1) % 10 + '0');  // 将数字转换为ASCII字符发送
	}
}

/**
 * @brief 重定向fputc函数，支持printf函数发送数据
 * @param ch 要发送的字符
 * @param f 文件指针
 * @retval 发送的字符
 */
int fputc(int ch, FILE *f){
	Serial_SendByte(ch);  // 调用串口发送函数
	return ch;
}

/**
 * @brief 串口printf函数，支持格式化输出
 * @param format 格式化字符串
 * @param ... 可变参数列表
 * @retval 无
 */
void Serial_Printf(char *format, ...){
	char String[100];  // 定义缓冲区
	va_list arg;       // 定义可变参数列表
	va_start(arg, format);  // 初始化可变参数列表
	vsprintf(String, format, arg);  // 将格式化字符串转换为字符串
	va_end(arg);  // 结束可变参数列表
	Serial_SendString(String);  // 发送字符串
} 

/**
 * @brief 发送数据包
 * @param 无
 * @retval 无
 * @note 数据包格式：0xFF + 4字节数据 + 0xFE
 */
void Serial_SendPacket(void){
	Serial_SendByte(0xFF);  // 发送帧头
	Serial_SendArray(Serial_TxPacket, 4);  // 发送数据包内容
	Serial_SendByte(0xFE);  // 发送帧尾
}

/**
 * @brief 获取接收标志
 * @param 无
 * @retval 1表示有新数据接收，0表示无新数据
 */
uint8_t Serial_GetRxFlag(void){
	if(Serial_RxFlag == 1){  // 检查接收标志
		Serial_RxFlag = 0;  // 清除接收标志
		return 1;  // 返回1表示有新数据
	}
	return 0;  // 返回0表示无新数据
}

/**
 * @brief USART1中断服务函数
 * @param 无
 * @retval 无
 * @note 接收数据包格式：0xFF + 4字节数据 + 0xFE
 */
void USART1_IRQHandler(void){
	static uint8_t RxState = 0;  // 接收状态机状态
	static uint8_t pRxPacket = 0;  // 接收数据包指针
		
	if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET){  // 检查是否为接收中断
		uint8_t RxData = USART_ReceiveData(USART1);  // 读取接收到的数据
		
		// 状态机处理接收数据
		if(RxState == 0){  // 等待帧头
			if(RxData == 0xFF){  // 接收到帧头
				RxState = 1;  // 进入接收数据状态
				pRxPacket = 0;  // 重置数据包指针
			}
		}
		else if(RxState == 1){  // 接收数据
			Serial_RxPacket[pRxPacket] = RxData;  // 存储接收到的数据
			pRxPacket++;  // 数据包指针递增
			if(pRxPacket >= 4){  // 接收完4字节数据
				RxState = 2;  // 进入等待帧尾状态
			}
		}
		else if(RxState == 2){  // 等待帧尾
			if(RxData == 0xFE){  // 接收到帧尾
				RxState = 0;  // 回到初始状态
				Serial_RxFlag = 1;  // 设置接收完成标志
			}
		}
		
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);  // 清除接收中断标志
	}
}