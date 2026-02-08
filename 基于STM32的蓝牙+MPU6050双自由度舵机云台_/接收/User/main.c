// ==================================================================
// MPU6050舵机云台接收端主程序
// 功能：接收发送端的角度数据，控制两个舵机平滑运动
// 作者：
// 日期：
// ==================================================================
#include "stm32f10x.h"                  // STM32F103系列头文件
#include "Delay.h"                      // 延时函数库
#include "OLED.h"                       // OLED显示库
#include "Serial.h"                     // 串口通信库
#include "Key.h"                        // 按键输入库
#include "Sundries.h"                   // 杂项功能库（角度解析、平滑控制）
#include "Servo.h"                      // 舵机控制库
#include <math.h>                       // 数学函数库

int main(void){

	OLED_Init();      // 初始化OLED显示屏
	Serial_Init();     // 初始化串口通信（波特率等设置）
    Servo_Init();      // 初始化舵机PWM控制

    // 在OLED上显示标题和标签
    OLED_ShowString(1,1,"Servo Ctrl:");  // 主标题
    OLED_ShowString(2,1,"A:");           // 舵机1角度标签
    OLED_ShowString(3,1,"B:");           // 舵机2角度标签

    // 舵机上电默认中间位置90°
    Servo_SetAngle1((uint16_t)servo1.current);  // 设置舵机1初始位置
    Servo_SetAngle2((uint16_t)servo2.current);  // 设置舵机2初始位置

	while(1){
		
        // 串口接收处理
        if (Serial_RxFlag == 1)  // 检查是否收到完整的角度帧
        {
            Parse_DualAngle();      // 解析接收到的角度数据
            Serial_RxFlag = 0;      // 重置接收标志，准备下次接收
        }

        // 舵机平滑控制
        Servo_SmoothControl();  // 根据目标角度和当前角度，平滑调整舵机位置（8ms/次更新）

        // OLED显示控制（16ms刷新一次）
        static uint8_t showCnt = 0;  // 显示计数器
        if(showCnt++ >= 2){
            OLED_ShowNum(2,3,(uint16_t)servo1.current,3);  // 显示舵机1当前角度
            OLED_ShowNum(3,3,(uint16_t)servo2.current,3);  // 显示舵机2当前角度
            showCnt = 0;  // 重置计数器
        }

        // 主循环延时（8ms，与发送端保持同步）
        Delay_ms(8);
		
	}

}