#include "stm32f10x.h"                  // Device header
#include "Sundries.h"
#include "Serial.h"

/**
 * @brief 外部变量声明
 */
extern float S1_Filtered, S2_Filtered;  // 滤波后的舵机角度值

/**
 * @brief 通过蓝牙发送双角度数据
 * @param 无
 * @retval 无
 * @note 发送格式：帧头(0xFF) + S1角度低8位 + S1角度高8位 + S2角度低8位 + S2角度高8位 + 帧尾(0xFE)
 */
void Bluetooth_Send_DualAngle(){
	// 将浮点角度值转换为整数（扩大10倍，保留一位小数精度）
	uint16_t s1_int = (uint16_t)(S1_Filtered * 10);
	uint16_t s2_int = (uint16_t)(S2_Filtered * 10);
	
	// 构建发送缓冲区
	uint8_t send_buf[6] = {
	   ANGLE_FRAME_HEADER,      // 帧头标记（0xFF）
	    (uint8_t)s1_int,         // S1角度低8位数据
	    (uint8_t)(s1_int >> 8),  // S1角度高8位数据
	    (uint8_t)s2_int,         // S2角度低8位数据
	    (uint8_t)(s2_int >> 8),  // S2角度高8位数据
	    ANGLE_FRAME_TAIL         // 帧尾标记（0xFE）
	};
	
	// 通过串口（蓝牙）发送数据数组
	Serial_SendArray(send_buf, 6);
}