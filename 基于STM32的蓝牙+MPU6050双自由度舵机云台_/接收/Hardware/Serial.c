// ==================================================================
// Serial.c - 串口通信模块
// 功能：
// 1. 初始化串口通信
// 2. 实现串口中断接收（角度帧接收）
// 3. 提供串口发送功能
// 作者：
// 日期：
// ==================================================================
#include "stm32f10x.h"                  // STM32F103系列头文件
#include <stdio.h>                       // 标准输入输出库
#include <stdarg.h>                      // 可变参数库

// 串口发送数据包（预留）
uint8_t Serial_TxPacket[4];

// 包含Sundries.h以使用FRAME_LENGTH宏定义
#include "Sundries.h"

// 串口接收缓存区（存储接收到的角度帧数据）
// 格式：[0xFF][s1_lower][s1_upper][s2_lower][s2_upper][0xFE]
uint8_t Serial_RxPacket[FRAME_LENGTH];

// 串口接收完成标志位（1: 接收完成，0: 未完成）
uint8_t Serial_RxFlag;

// ==================================================================
// 函数名：Serial_Init
// 功能：初始化串口通信（USART1）
// 参数：无
// 返回值：无
// 说明：配置GPIO引脚、串口参数、中断优先级等
// ==================================================================
void Serial_Init(void){

	// 使能串口1和GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	// 配置PA9为串口发送引脚（复用推挽输出）
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;          // 复用推挽输出模式
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9;                // PA9引脚
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;        // 50MHz输出速度
	GPIO_Init(GPIOA,&GPIO_InitStructure);                  // 应用配置
	
	// 配置PA10为串口接收引脚（上拉输入）
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;            // 上拉输入模式
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;               // PA10引脚
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;        // 50MHz输入速度
	GPIO_Init(GPIOA,&GPIO_InitStructure);                  // 应用配置
	
	// 配置串口参数
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate=9600;                    // 波特率：9600
	USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;  // 无硬件流控
	USART_InitStructure.USART_Mode=USART_Mode_Tx|USART_Mode_Rx;  // 同时启用发送和接收
	USART_InitStructure.USART_Parity=USART_Parity_No;            // 无校验位
	USART_InitStructure.USART_StopBits=USART_StopBits_1;         // 1位停止位
	USART_InitStructure.USART_WordLength=USART_WordLength_8b;    // 8位数据位
	USART_Init(USART1,&USART_InitStructure);                     // 应用配置
	
	// 使能串口接收中断
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	
	// 配置中断优先级分组
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	// 配置串口中断
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel=USART1_IRQn;          // USART1中断通道
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;            // 使能中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;   // 抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;          // 子优先级1
	NVIC_Init(&NVIC_InitStructure);                           // 应用配置
	
	// 使能串口1
	USART_Cmd(USART1,ENABLE);
}

void Serial_SendByte(uint8_t Byte){

	USART_SendData(USART1,Byte);
	while (USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);

}

void Serial_SendArray(uint8_t *Array,uint16_t Length){

	uint16_t i;
	for(i=0;i<Length;i++){
		Serial_SendByte(Array[i]);
	}

}

void Serial_SendString(char *String){

	uint8_t i;
	for(i=0;String[i] != '\0';i++){
		Serial_SendByte(String[i]);
	}

}

uint32_t Serial_Pow(uint32_t X,uint32_t Y){

	uint32_t Result = 1;
	while(Y--){
		Result *= X;
	}
	return Result;
}

void Serial_SendNumber(uint32_t Number,uint8_t Length){

	uint8_t i;
	for(i=0;i<Length;i++){
		Serial_SendByte(Number / Serial_Pow(10,Length - i - 1)%10 + '0');
	}

}

int fputc(int ch,FILE *f){

	Serial_SendByte(ch);
	return ch;
}

void Serial_Printf(char *format,...){

	char String[100];
	va_list arg;
	va_start(arg,format);
	vsprintf(String,format,arg);
	va_end(arg);
	Serial_SendString(String);
} 

void Serial_SendPacket(void){

	Serial_SendByte(0xFF);
	Serial_SendArray(Serial_TxPacket,4);
	Serial_SendByte(0xFE);

}

uint8_t Serial_GetRxFlag(void){

	if(Serial_RxFlag == 1){
		Serial_RxFlag=0;
		return 1;
	}
	return 0;
}


// ==================================================================
// 函数名：USART1_IRQHandler
// 功能：USART1串口中断服务函数
// 参数：无
// 返回值：无
// 说明：实现串口接收状态机，接收6字节角度帧数据
// 数据格式：[0xFF][s1_lower][s1_upper][s2_lower][s2_upper][0xFE]
// ==================================================================
void USART1_IRQHandler(void){

	static uint8_t RxState=0;     // 接收状态机状态（0:等待帧头, 1:接收数据, 2:等待帧尾）
	static uint8_t pRxPacket=0;   // 接收数据计数指针
		
	// 检查是否为接收中断
	if(USART_GetITStatus(USART1,USART_IT_RXNE) == SET){

		uint8_t RxData = USART_ReceiveData(USART1);  // 读取接收到的数据
		
		// 状态0：等待帧头0xFF
		if(RxState==0){
			if(RxData==0xFF){
				RxState=1;        // 收到帧头，进入状态1
				pRxPacket=0;      // 重置接收指针
			}
		}
		// 状态1：接收数据字节（共4字节角度数据）
		else if(RxState==1){
			Serial_RxPacket[pRxPacket]=RxData;  // 存储接收到的数据
			pRxPacket++;                        // 指针递增
			if(pRxPacket>=4){                   // 接收4字节后进入状态2
				RxState=2;
			}
		}
		// 状态2：等待帧尾0xFE
		else if(RxState==2){
			if(RxData==0xFE){
				RxState=0;            // 收到帧尾，回到状态0
				Serial_RxFlag=1;      // 设置接收完成标志
			}
		}
		
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);  // 清除中断标志位
	}

}