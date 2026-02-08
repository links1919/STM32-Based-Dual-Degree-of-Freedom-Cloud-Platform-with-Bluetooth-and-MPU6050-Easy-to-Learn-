#ifndef __SUNDRIES_H__
#define __SUNDRIES_H__

// 舵机角度范围（与发送端严格匹配）
#define SERVO1_MIN      30.0f        
#define SERVO1_MAX     150.0f        
#define SERVO2_MIN       0.0f        
#define SERVO2_MAX     180.0f  

#define FRAME_LENGTH       6     // 角度帧固定长度：1字节头 + 2字节S1 + 2字节S2 + 1字节尾
#define ANGLE_FRAME_HEADER 0xFF      // 角度帧头
#define ANGLE_FRAME_TAIL   0xFE      // 角度帧尾

#define SMALL_ANGLE      5.0f        // 小角度阈值
#define LARGE_ANGLE     15.0f        // 大角度阈值
#define SMALL_STEP       0.8f        // 小步长（微调无抖动）
#define LARGE_STEP       7.0f        // 大步长（快速到位）

typedef struct {
    float target;       // 目标角度（从发送端解析得到）
    float current;      // 当前角度（舵机实际位置，用于平滑控制）
    float min;          // 最小角度限制
    float max;          // 最大角度限制
} ServoState;

// 外部变量声明
extern ServoState servo1;
extern ServoState servo2;

// 函数声明
void Parse_DualAngle(void);
void Servo_SmoothControl(void);

#endif