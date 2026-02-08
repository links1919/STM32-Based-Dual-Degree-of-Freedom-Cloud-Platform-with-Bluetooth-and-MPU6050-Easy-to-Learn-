#ifndef _SUNDRIES_H
#define _SUNDRIES_H

/**
 * @brief 角度范围定义
 * @note 有效角度范围为±ANGLE_RANGE度
 */
#define ANGLE_RANGE 30.0f

/**
 * @brief 蓝牙通信帧格式定义
 */
#define ANGLE_FRAME_HEADER 0xFF  // 角度数据帧头标记
#define ANGLE_FRAME_TAIL 0xFE    // 角度数据帧尾标记

/**
 * @brief 舵机1角度范围定义
 */
#define SERVO1_MIN 30.0f         // 舵机1最小角度（度）
#define SERVO1_MAX 150.0f        // 舵机1最大角度（度）

/**
 * @brief 舵机2角度范围定义
 */
#define SERVO2_MIN 0.0f          // 舵机2最小角度（度）
#define SERVO2_MAX 180.0f        // 舵机2最大角度（度）

/**
 * @brief 滤波系数定义
 * @note FILTER_ALPHA越大，响应越快但滤波效果越差；反之亦然
 */
#define FILTER_ALPHA 0.8f

/**
 * @brief 主循环间隔时间
 * @note 单位：毫秒，需与接收端保持同步
 */
#define LOOP_INTERVAL 8

/**
 * @brief 函数声明
 */
void Bluetooth_Send_DualAngle();  // 通过蓝牙发送双角度数据

#endif