#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "Key.h"
#include "Sundries.h"
#include "MyI2C.h"
#include "MPU6050.h"
#include <math.h>

/**
 * @brief 全局变量定义
 */
uint8_t KeyNum;              // 按键编号
uint8_t ID;                  // MPU6050设备ID
int16_t AX, AY, AZ;          // 加速度计原始数据（X、Y、Z轴）
float AX_g, AY_g, AZ_g;      // 加速度计数据（单位：g）
float ThetaX, ThetaY;        // 计算得到的角度（X、Y轴）
float S1_Angle, S2_Angle;    // 舵机目标角度
float S1_Filtered, S2_Filtered;  // 滤波后的舵机角度

/**
 * @brief 主函数
 * @param 无
 * @retval 无
 */
int main(void){
	// 初始化各个模块
	OLED_Init();        // 初始化OLED显示屏
	MPU6050_Init();     // 初始化MPU6050传感器
	Serial_Init();      // 初始化串口通信（蓝牙）
	
	// 在OLED上显示初始信息
	OLED_ShowString(1, 1, "ID:");       // 显示ID标签
	OLED_ShowString(2, 1, "MPU6050");   // 显示MPU6050标识
	OLED_ShowString(3, 1, "X:");        // 显示X轴角度标签
	OLED_ShowString(4, 1, "Y:");        // 显示Y轴角度标签
	
	// 获取并显示MPU6050的设备ID
	ID = MPU6050_GetID();           // 读取MPU6050的设备ID
	OLED_ShowHexNum(1, 4, ID, 2);   // 在OLED上显示设备ID（十六进制）
	
	// 初始校准：读取初始加速度数据
	MPU6050_GetData(&AX, &AY, &AZ, 0, 0, 0);  // 只读取加速度数据
	
	// 将原始加速度数据转换为单位为g的值（±16g量程时，灵敏度为32767/16=2048LSB/g）
	AX_g = (float)AX * 32 / 65535;  // 转换X轴加速度（32 = 2*16g量程）
    AY_g = (float)AY * 32 / 65535;  // 转换Y轴加速度
    AZ_g = (float)AZ * 32 / 65535;  // 转换Z轴加速度
	
	// 防止除零错误：如果AZ_g过小，设置一个较小的值
	if(fabs(AZ_g) < 0.1f){
		AZ_g = (AZ_g > 0) ? 0.1f : -0.1f;
	}
	
	// 使用反正切函数计算X和Y轴的角度
	ThetaX = atan(AX_g / AZ_g) * 180 / 3.14159;  // 计算X轴角度（弧度转角度）
	ThetaY = atan(AY_g / AZ_g) * 180 / 3.14159;  // 计算Y轴角度
	
	// 将角度转换为舵机的PWM值
	S1_Angle = ((ThetaX + ANGLE_RANGE) / (2 * ANGLE_RANGE)) * (SERVO1_MAX - SERVO1_MIN) + SERVO1_MIN;
	S2_Angle = ((ThetaY + ANGLE_RANGE) / (2 * ANGLE_RANGE)) * (SERVO2_MAX - SERVO2_MIN) + SERVO2_MIN;
	
	// 初始化滤波后的角度
	S1_Filtered = S1_Angle;  // 初始滤波值为当前角度
	S2_Filtered = S2_Angle;
	
	// 主循环
	while(1){
		// 读取MPU6050的加速度数据
		MPU6050_GetData(&AX, &AY, &AZ, 0, 0, 0);  // 只读取加速度数据
		
		// 将原始加速度数据转换为单位为g的值
		AX_g = (float)AX * 32 / 65535;
        AY_g = (float)AY * 32 / 65535;
        AZ_g = (float)AZ * 32 / 65535;
		
		// 防止除零错误
		if(fabs(AZ_g) < 0.1f){
			AZ_g = (AZ_g > 0) ? 0.1f : -0.1f;
		}
		
		// 计算角度
		ThetaX = atan(AX_g / AZ_g) * 180 / 3.14159;
		ThetaY = atan(AY_g / AZ_g) * 180 / 3.14159;
		
		// 限制角度范围
		ThetaX = (ThetaX < -ANGLE_RANGE) ? -ANGLE_RANGE : ((ThetaX > ANGLE_RANGE) ? ANGLE_RANGE : ThetaX);
		ThetaY = (ThetaY < -ANGLE_RANGE) ? -ANGLE_RANGE : ((ThetaY > ANGLE_RANGE) ? ANGLE_RANGE : ThetaY);
		
		// 将角度转换为舵机的PWM值
		S1_Angle = ((ThetaX + ANGLE_RANGE) / (2 * ANGLE_RANGE)) * (SERVO1_MAX - SERVO1_MIN) + SERVO1_MIN;
        S2_Angle = ((ThetaY + ANGLE_RANGE) / (2 * ANGLE_RANGE)) * (SERVO2_MAX - SERVO2_MIN) + SERVO2_MIN;
		
		// 限制舵机角度范围
		S1_Angle = (S1_Angle < SERVO1_MIN) ? SERVO1_MIN : (S1_Angle > SERVO1_MAX ? SERVO1_MAX : S1_Angle);
        S2_Angle = (S2_Angle < SERVO2_MIN) ? SERVO2_MIN : (S2_Angle > SERVO2_MAX ? SERVO2_MAX : S2_Angle);
		
		// 使用一阶低通滤波平滑舵机角度
		S1_Filtered = S1_Angle * FILTER_ALPHA + S1_Filtered * (1 - FILTER_ALPHA);
		S2_Filtered = S2_Angle * FILTER_ALPHA + S2_Filtered * (1 - FILTER_ALPHA);
		
		// 通过蓝牙发送双角度数据
		Bluetooth_Send_DualAngle();
		
		// 每隔一段时间更新OLED显示（降低显示频率，减少资源占用）
		static uint8_t showCnt = 0;
		if(showCnt >= 2){
			OLED_ShowNum(3, 3, (uint16_t)S1_Filtered, 3);  // 显示X轴角度
			OLED_ShowNum(4, 3, (uint16_t)S2_Filtered, 3);  // 显示Y轴角度
			showCnt = 0;  // 重置计数器
		}
		else{
			showCnt++;  // 计数器递增
		}
		
		Delay_ms(LOOP_INTERVAL);  // 循环间隔延时
	}
}