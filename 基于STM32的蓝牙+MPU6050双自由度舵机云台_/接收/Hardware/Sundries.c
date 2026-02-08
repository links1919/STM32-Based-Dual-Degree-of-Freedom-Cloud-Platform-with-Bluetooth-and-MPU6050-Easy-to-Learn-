// ==================================================================
// Sundries.c - 杂项功能模块
// 功能：
// 1. 角度数据解析
// 2. 舵机状态管理
// 3. 平滑控制算法
// 作者：
// 日期：
// ==================================================================
#include "stm32f10x.h"                  // STM32F103系列头文件
#include "Sundries.h"                   // 自身头文件，包含结构体定义和宏定义
#include <math.h>                       // 数学函数库
#include "PWM.h"                       // PWM控制库
#include "Servo.h"                      // 舵机控制库
#include "Serial.h"                     // 串口通信库

/* typedef struct {
    float target;       // 目标角度（从发送端解析得到）
    float current;      // 当前角度（舵机实际位置，用于平滑控制）
    float min;          // 最小角度限制
    float max;          // 最大角度限制
} ServoState;
 */

// 初始化舵机状态（上电默认中间位置：舵机1=90°，舵机2=90°）
ServoState servo1 = {90.0f,90.0f,SERVO1_MIN,SERVO1_MAX};  // 舵机1状态：目标90°，当前90°
ServoState servo2 = {90.0f,90.0f,SERVO2_MIN,SERVO2_MAX};  // 舵机2状态：目标90°，当前90°


// ==================================================================
// 函数名：Parse_DualAngle
// 功能：解析从发送端接收的双舵机角度数据帧
// 参数：无
// 返回值：无
// 说明：将接收到的16位角度数据转换为浮点数角度值（0.1°精度）
// ==================================================================
// ==================================================================
// 函数名：Parse_DualAngle
// 功能：解析发送端发送的双舵机角度帧数据
// 参数：无
// 返回值：无
// 说明：从串口接收的数据包中解析出两个舵机的目标角度，并进行范围保护
// ==================================================================
void Parse_DualAngle(){

    
    // 拼接16位角度数据（发送端以0.1°精度发送，高字节在前，低字节在后）
    uint16_t s1_int = Serial_RxPacket[0] | (Serial_RxPacket[1] << 8);  // 舵机1角度值（16位）
    uint16_t s2_int = Serial_RxPacket[2] | (Serial_RxPacket[3] << 8);  // 舵机2角度值（16位）

    // 转换为浮点角度值（0.1°精度转换为1°精度）
    servo1.target = (float)s1_int / 10.0f;  // 舵机1目标角度
    servo2.target = (float)s2_int / 10.0f;  // 舵机2目标角度

    // 角度范围保护
    servo1.target = 180 - ((servo1.target < servo1.min) ? servo1.min :((servo1.target > servo1.max) ? servo1.max : servo1.target));
    servo2.target = 180 - ((servo2.target < servo2.min) ? servo2.min :((servo2.target > servo2.max) ? servo2.max : servo2.target));
    

}

// ==================================================================
// 函数名：CalculateStep
// 功能：根据角度差计算平滑控制的步进值
// 参数：absDiff - 当前角度与目标角度的绝对值差
// 返回值：计算得到的步进值
// 说明：实现非线性步进控制，角度差小时使用小步长（避免抖动），角度差大时使用大步长（快速响应）
// ==================================================================
static float CalculateStep(float absDiff){

    // 小角度差（微调），使用小步长
    if(absDiff <= SMALL_ANGLE){
        return SMALL_STEP;
    }
    // 中等角度差，使用线性插值的步长（平滑过渡）
    else if(absDiff <= LARGE_ANGLE){
        return SMALL_STEP + (LARGE_STEP - SMALL_STEP) * (absDiff - SMALL_ANGLE) / (LARGE_ANGLE - SMALL_ANGLE);
    }
    // 大角度差（快速调整），使用大步长
    else{
        return LARGE_STEP;
    }
}

// ==================================================================
// 函数名：Servo_SmoothControl
// 功能：双舵机平滑控制主函数
// 参数：无
// 返回值：无
// 说明：根据目标角度和当前角度的差值，计算平滑的步进值，实现舵机的无抖动运动
// ==================================================================
void Servo_SmoothControl(){

    // 舵机1平滑控制
    float diff1 = servo1.target - servo1.current;  // 计算角度差（目标角度 - 当前角度）
    float step1 = CalculateStep(fabs(diff1));      // 根据角度差绝对值计算步进值

    if(diff1 > step1){
        servo1.current += step1;  // 正向调整角度
    }
    else if(diff1 < -step1){
        servo1.current -= step1;  // 反向调整角度
    }
    else{
        servo1.current = servo1.target;  // 角度差小于最小步长，直接到达目标
    }

    // 舵机2平滑控制（与舵机1控制逻辑相同）
    float diff2 = servo2.target - servo2.current;  // 计算角度差
    float step2 = CalculateStep(fabs(diff2));      // 根据角度差绝对值计算步进值

    if(diff2 > step2){
        servo2.current += step2;  // 正向调整角度
    }
    else if(diff2 < -step2){
        servo2.current -= step2;  // 反向调整角度
    }
    else{
        servo2.current = servo2.target;  // 角度差小于最小步长，直接到达目标
    }

    // 将计算得到的角度值应用到实际舵机
    Servo_SetAngle1((uint16_t)servo1.current);  // 设置舵机1当前位置
    Servo_SetAngle2((uint16_t)servo2.current);  // 设置舵机2当前位置

}