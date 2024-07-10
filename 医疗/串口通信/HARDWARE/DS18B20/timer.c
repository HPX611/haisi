#include "timer.h"
#include "led.h"
#include "usart.h"
#include "dht11.h"
#include "wifi.h"
#include <string.h>
 
 

//通用定时器中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器3!
void TIM3_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能

	TIM_TimeBaseStructure.TIM_Period = arr; //10000-1 = 9999 设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到5000为500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //36000-1 = 35999 设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
 //TimeOut = （（PSC+ 1） * （ARR+ 1） ） / TIMxCLK 单位秒 
	TIM_ITConfig(  //使能或者失能指定的TIM中断
		TIM3, //TIM2
		TIM_IT_Update ,
		ENABLE  //使能
		);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

	TIM_Cmd(TIM3, ENABLE);  //使能TIMx外设
							 
}

char humi,temp1,buf2[50];
extern unsigned int rec_data[4];
extern u8 value[3];
extern uint32_t tiwen;

void TIM3_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
	{
		
		LED0=!LED0;
		
		//DHT11_Read_Data(&temp1,&humi);
		//printf("temp:%d%d,humi:%d%d\r\n",temp/10,temp%10,humi/10,humi%10);
		sprintf(buf2,"AT+MQTTPUB=0,\"dbld_data\",\"{temp:%d.%d\\\,humi:%d.%d\\\,tiwen:%d\\\,xl&xy:%d&%d}\",0,0",rec_data[0]+26,rec_data[1],rec_data[2]%100,rec_data[3],tiwen,value[0],value[1]);
		printf("buf2:%s\r\n",buf2);
		wifi_send_cmd(buf2,"OK",100);
		
	  TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx的中断待处理位:TIM 中断源 
	}
}












