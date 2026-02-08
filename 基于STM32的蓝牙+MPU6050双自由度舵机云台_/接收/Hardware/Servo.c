#include "stm32f10x.h"                  // Device header
#include "PWM.h"


void Servo_Init(void)
{
	PWM_Init();									//初始化舵机的底层PWM
}


  
void Servo_SetAngle1(float Angle1)
{
	PWM_SetCompare1(Angle1 / 180 * 2000 + 500);	//设置占空比
												//将角度线性变换，对应到舵机要求的占空比范围上
}

void Servo_SetAngle2(float Angle2)
{
	PWM_SetCompare2(Angle2 / 180 * 2000 + 500);	//设置占空比
												//将角度线性变换，对应到舵机要求的占空比范围上
}