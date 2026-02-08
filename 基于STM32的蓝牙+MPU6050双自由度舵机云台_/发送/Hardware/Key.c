#include "stm32f10x.h"                  // Device header
#include "Delay.h"

/**
 * @brief 按键初始化函数
 * @param 无
 * @retval 无
 * @note 配置PB1和PB11为上拉输入模式
 */
void Key_Init(void){
	// 使能GPIOB时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	// 配置GPIO引脚
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 上拉输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_11;  // PB1和PB11引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  // 引脚速度50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);  // 初始化GPIOB
}

/**
 * @brief 获取按键状态
 * @param 无
 * @retval 按键编号（0表示无按键按下，1表示按键1按下，2表示按键2按下）
 * @note 实现了按键消抖功能
 */
uint8_t Key_GetNum(void){
	uint8_t KeyNum = 0;  // 按键编号，初始值为0（无按键按下）
	
	// 检测按键1（PB1）是否按下
	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0){
		Delay_ms(20);  // 延时20ms消抖
		while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0);  // 等待按键释放
		Delay_ms(20);  // 延时20ms消抖
		KeyNum = 1;  // 设置按键编号为1
	}
	
	// 检测按键2（PB11）是否按下
	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0){
		Delay_ms(20);  // 延时20ms消抖
		while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0);  // 等待按键释放
		Delay_ms(20);  // 延时20ms消抖
		KeyNum = 2;  // 设置按键编号为2
	}
	
	return KeyNum;  // 返回按键编号
}