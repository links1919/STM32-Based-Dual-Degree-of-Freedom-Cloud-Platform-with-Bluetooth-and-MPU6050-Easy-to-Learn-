# 基于STM32的蓝牙+MPU6050双自由度舵机云台
  本项目基于B站江科大的模块代码，方便学习和移植，所有代码均已注释，可放心查看。推荐搭配网课和AI辅助学习。代码分为两个部分，一块单片机上烧录发送部分，另一块单片机上烧录接收部分。  

    
 材料：两块STM32F103C8T6,两块OLED显示屏，两个型号SG90的180°舵机，两个HC-05主从一体兼容版蓝牙模块（不差钱可以用HC-04，B站江科大有教程），两块面包板，一个二自由度舵机云台塑料支架（淘宝上很多），一块MPU6050,以及ST—-Link烧录器，USB转串口模块（例如USB转TTL模块），面包板供电模块和若干杜邦线。  
  
  注意事项：HC-05需一个设置为主模式（发送部分），另一个设置为从模式（接收部分），B站上可以搜到教程。舵机最好要单独供电，与单片机和蓝牙的供电分开，两者共地即可（负极相接）。不然可能会因舵机启动时造成的电压不稳，而导致蓝牙重启断开连接或单片机损坏。  
  
主要接线：  
    发送部分：MPU6050:SDA-->PB1,SCL-->PB10,GND-->GND,VCC-->3.3V  
    &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp;&nbsp;
    HC-05:RXD-->PA9,TXD-->PA10,GND-->GND,VCC-->5V  
    接收部分：舵机1（上面的）：GND-->GND,VCC-->5V，信号线--PA0  
    &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp;&nbsp;
    舵机2（下面的）：GND-->GND,VCC-->5V，信号线--PA1  
    &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp;&nbsp;
    HC-05:RXD-->PA9,TXD-->PA10,GND-->GND,VCC-->5V
    
