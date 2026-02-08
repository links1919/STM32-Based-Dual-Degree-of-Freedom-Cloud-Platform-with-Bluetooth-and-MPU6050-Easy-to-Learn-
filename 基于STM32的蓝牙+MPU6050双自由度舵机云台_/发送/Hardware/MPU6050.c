#include "stm32f10x.h"                  // Device header
#include "MyI2C.h"
#include "MPU6050_Reg.h"

/**
 * @brief MPU6050设备地址
 * @note 7位地址为0x68，左移1位后为0xD0（写操作地址）
 */
#define MPU6050_ADDRESS		0xD0

/**
 * @brief MPU6050写寄存器函数
 * @param RegAddress 寄存器地址
 * @param Data 要写入的数据
 * @retval 无
 */
void MPU6050_WriteReg(uint8_t RegAddress,uint8_t Data){
	MyI2C_Start();                  // 发送I2C开始信号
	MyI2C_SendByte(MPU6050_ADDRESS); // 发送MPU6050设备地址（写操作）
	MyI2C_ReceiveAck();             // 接收应答
	MyI2C_SendByte(RegAddress);     // 发送寄存器地址
	MyI2C_ReceiveAck();             // 接收应答
	MyI2C_SendByte(Data);           // 发送要写入的数据
	MyI2C_ReceiveAck();             // 接收应答
	MyI2C_Stop();                   // 发送I2C停止信号
}

/**
 * @brief MPU6050读寄存器函数
 * @param RegAddress 寄存器地址
 * @retval 读取到的数据
 */
uint8_t MPU6050_ReadReg(uint8_t RegAddress){
	uint8_t Data;
	
	MyI2C_Start();                  // 发送I2C开始信号
	MyI2C_SendByte(MPU6050_ADDRESS); // 发送MPU6050设备地址（写操作）
	MyI2C_ReceiveAck();             // 接收应答
	MyI2C_SendByte(RegAddress);     // 发送要读取的寄存器地址
	MyI2C_ReceiveAck();             // 接收应答
	
	MyI2C_Start();                  // 发送I2C重复开始信号
	MyI2C_SendByte(MPU6050_ADDRESS | 0x01); // 发送MPU6050设备地址（读操作）
	MyI2C_ReceiveAck();             // 接收应答
	Data = MyI2C_ReceiveByte();     // 读取数据
	MyI2C_SendAck(1);               // 发送非应答信号（结束读取）
	MyI2C_Stop();                   // 发送I2C停止信号
	
	return Data;                    // 返回读取到的数据
}

/**
 * @brief MPU6050初始化函数
 * @param 无
 * @retval 无
 */
void MPU6050_Init(void){
	MyI2C_Init();                          // 初始化I2C总线
	MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x01);   // 电源管理寄存器1：设置使用X轴陀螺仪作为时钟源
	MPU6050_WriteReg(MPU6050_PWR_MGMT_2, 0x00);   // 电源管理寄存器2：所有轴都不待机
	MPU6050_WriteReg(MPU6050_SMPLRT_DIV, 0x09);   // 采样率分频器：设置采样率为100Hz
	MPU6050_WriteReg(MPU6050_CONFIG, 0x06);       // 配置寄存器：DLPF带宽为5Hz
	MPU6050_WriteReg(MPU6050_GYRO_CONFIG, 0x18);  // 陀螺仪配置寄存器：选择±2000°/s的满量程范围
	MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, 0x18); // 加速度计配置寄存器：选择±16g的满量程范围
}

/**
 * @brief 获取MPU6050的设备ID
 * @param 无
 * @retval 设备ID
 */
uint8_t MPU6050_GetID(void){
	return MPU6050_ReadReg(MPU6050_WHO_AM_I);  // 读取WHO_AM_I寄存器的值
}

/**
 * @brief 获取MPU6050的传感器数据
 * @param AccX 指向X轴加速度数据的指针
 * @param AccY 指向Y轴加速度数据的指针
 * @param AccZ 指向Z轴加速度数据的指针
 * @param GyroX 指向X轴陀螺仪数据的指针
 * @param GyroY 指向Y轴陀螺仪数据的指针
 * @param GyroZ 指向Z轴陀螺仪数据的指针
 * @retval 无
 */
void MPU6050_GetData(int16_t *AccX,int16_t *AccY,int16_t *AccZ,
			 int16_t *GyroX,int16_t *GyroY,int16_t *GyroZ)
{
	uint8_t DataH, DataL;  // 用于存储高字节和低字节数据
	
	// 读取X轴加速度数据
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_H);  // 读取高字节
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_L);  // 读取低字节
	*AccX = (DataH << 8) | DataL;  // 组合成16位数据
	
	// 读取Y轴加速度数据
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_H);  // 读取高字节
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_L);  // 读取低字节
	*AccY = (DataH << 8) | DataL;  // 组合成16位数据
	
	// 读取Z轴加速度数据
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_H);  // 读取高字节
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_L);  // 读取低字节
	*AccZ = (DataH << 8) | DataL;  // 组合成16位数据
	
	// 读取X轴陀螺仪数据
	DataH = MPU6050_ReadReg(MPU6050_GYRO_XOUT_H);  // 读取高字节
	DataL = MPU6050_ReadReg(MPU6050_GYRO_XOUT_L);  // 读取低字节
	*GyroX = (DataH << 8) | DataL;  // 组合成16位数据
	
	// 读取Y轴陀螺仪数据
	DataH = MPU6050_ReadReg(MPU6050_GYRO_YOUT_H);  // 读取高字节
	DataL = MPU6050_ReadReg(MPU6050_GYRO_YOUT_L);  // 读取低字节
	*GyroY = (DataH << 8) | DataL;  // 组合成16位数据
	
	// 读取Z轴陀螺仪数据
	DataH = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_H);  // 读取高字节
	DataL = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_L);  // 读取低字节
	*GyroZ = (DataH << 8) | DataL;  // 组合成16位数据
}