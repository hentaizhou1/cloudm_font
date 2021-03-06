#include "sys.h"
#include "motor_Contral.h"
#include "usart3.h"
#include "usart.h"	
#include "delay.h"

/*==========================================================   
 *函数名称：MotorInit()
 *功    能：对电机控制的IO口进行初始化
 *入口参数：无
 *出口参数：无
 *说    明：
 *==========================================================*/
 
 
 void MotorInit(void){
	 
	GPIO_InitTypeDef GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOA,ENABLE);//使能PORTB,PORTE时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);	// 关闭PB4 JTAG功能 复用为普通io口

	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_7|GPIO_Pin_6|GPIO_Pin_5|GPIO_Pin_4;//电机控制输出  1~4
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //设置成推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB 7-4
	GPIO_ResetBits(GPIOB,GPIO_Pin_4);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_6|GPIO_Pin_5|GPIO_Pin_4;//电机控制输出  5~8
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //设置成推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA 7-4
	 
	/*============================================================================================*/
	 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);//使能PORTE,PORTE时钟
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1;		//外部中断读取电机io返回信号 PC1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成下拉输入
	GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOC1  
 }
 
 
 
 /*==========================================================   
 *函数名称：ExtixInit()
 *功    能：对电机控制的IO口的外部中断进行初始化
 *入口参数：无
 *出口参数：无
 *说    明：`
 *==========================================================*/
 
 
void ExtixInit(void){
	 
	EXTI_InitTypeDef EXTI_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//使能复用功能时钟
	MotorInit();		//电机引脚初始化
	 
	//GPIOC.1 中断线以及中断初始化配置   下降沿触发
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource1);
	 
	EXTI_InitStructure.EXTI_Line=EXTI_Line1;	//motor  input
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;		//下降沿触发中断
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	 	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器  1
 
 }
 
 /*==========================================================   
 *函数名称：void EXTI_SetupNVIC(void)
 *功    能：初始化电机信号口中断优先级
 *入口参数：无
 *出口参数：无
 *说    明：`
 *==========================================================*/
 
 
void EXTI_SetupNVIC(void){
	
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;			//使能外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;	//抢占优先级2， 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;					//子优先级2
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure);  
 
 }

 
 /*==========================================================   
 *函数名称：void Exit_Setup(void)
 *功    能：总初始化
 *入口参数：无
 *出口参数：无
 *说    明：`
 *==========================================================*/
 
void ExitSetup(void){
	 
	ExtixInit();	//外部中断初始化
	EXTI_SetupNVIC();	//中断优先级初始化
	 
}


//外部中断1服务程序  PC1
int counter = 0;

u8 PushDrugs[5][2];
u32 pushDrugsNumbers;
u32 drugsNum_flag = 66;  // 出货数量的标志 
u16 getDrugsMsg_flag = 1;  // 药品选择标志位  选择一次后等待出货完毕才进行下一次选择

void EXTI1_IRQHandler(void){
	
	delay_ms(3);
	if(PCin(1) == 0){
		drugsNum_flag--;
		if(drugsNum_flag <= 0) {
				
				drugsNum_flag = 66;
				GPIOA->BRR = 0x00F0;		//关闭所有电机     io寄存器操作参考自：https://blog.csdn.net/stephenbruce/article/details/48392769    http://blog.sina.com.cn/s/blog_640029b30100i33h.html
				GPIOB->BRR = 0X00F0;
				getDrugsMsg_flag = 0;
		}
	}
	
//	if(PCin(1) == 0) {
//		counter++;
//		printf("ok");
//		GPIOA->BRR = 0x00F0;		//关闭所有电机     io寄存器操作参考自：https://blog.csdn.net/stephenbruce/article/details/48392769    http://blog.sina.com.cn/s/blog_640029b30100i33h.html
//		GPIOB->BRR = 0X00F0;
//	}
	
	EXTI_ClearITPendingBit(EXTI_Line1); //清除LINE1上的中断标志位  
}


