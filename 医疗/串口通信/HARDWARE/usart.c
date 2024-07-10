#include "usart.h"
#include "sys.h"
#include "stdio.h"


#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

//#if EN_USART_RX//如果串口接受使能

#define USART_RX_LEN 200
uint16_t USART_RX_BUF[USART_REC_LEN];
uint16_t USART_RX_BUF3[USART_REC_LEN];
u16 USART_RX_STA=0;
u16 USART_RX_STA3=0;

void uart_init(u32 bound)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_AFIO,ENABLE);//使能GPIO引脚时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//使能串口时钟
	
	GPIO_InitTypeDef GPIO_InitTypeStruct;//定义GPIO结构体变量
	USART_InitTypeDef USART_InitTypeStruct;//定义串口结构体变量
	NVIC_InitTypeDef NVIC_InitTypeStruct;//定义NVIC结构体变量
	
	GPIO_InitTypeStruct.GPIO_Mode=GPIO_Mode_AF_PP;//初始化GPIO引脚A9及模式
	GPIO_InitTypeStruct.GPIO_Pin=GPIO_Pin_9;
	GPIO_InitTypeStruct.GPIO_Speed =GPIO_Speed_10MHz;
	GPIO_Init(GPIOA,&GPIO_InitTypeStruct);
	
	GPIO_InitTypeStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;//初始化GPIO引脚A10及模式
	GPIO_InitTypeStruct.GPIO_Pin=GPIO_Pin_10;
	GPIO_InitTypeStruct.GPIO_Speed =GPIO_Speed_10MHz;
	GPIO_Init(GPIOA,&GPIO_InitTypeStruct);
	
	USART_InitTypeStruct.USART_BaudRate=bound;//初始化串口USART1
	USART_InitTypeStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitTypeStruct.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
	USART_InitTypeStruct.USART_Parity=USART_Parity_No;
	USART_InitTypeStruct.USART_StopBits=USART_StopBits_1;
	USART_InitTypeStruct.USART_WordLength=USART_WordLength_8b;
	USART_Init(USART1,&USART_InitTypeStruct);
	

	
	NVIC_InitTypeStruct.NVIC_IRQChannel=USART1_IRQn;//初始化中断配置
	NVIC_InitTypeStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitTypeStruct.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitTypeStruct.NVIC_IRQChannelSubPriority=1;
	NVIC_Init(&NVIC_InitTypeStruct);
	
	USART_Cmd(USART1,ENABLE);//使能串口（打开串口）
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);//开启接受中断（接收缓存器非空，则中断开启）
	
}
void USART1_IRQHandler(void)//中断服务函数
{
	u8 Res;
	if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET)//判断该中断为接收中断
	{
		Res=USART_ReceiveData(USART1);//读取接受信息
	//	printf("%d\n",Res);
		if((USART_RX_STA&0x8000)==0)//未接收到0x0a
		{
			if(USART_RX_STA&0x4000)//未接收到0x0a，接收到0x0d
			{
				if(Res==0x0a)USART_RX_STA|=0x8000;//接收数据为0x0a,接收完成
				else USART_RX_STA=0;//否则数据接收错误，重新接收
			}
			else//未接收到0x0a,未接收到0x0d
			{
				if(Res==0x0d)//接收数据为0x0d
					USART_RX_STA|=0x4000;
				else //未接收到0x0a,接收数据不为0x0d（在接受数据中）
				{
					USART_RX_BUF[USART_RX_STA&0x3FFF]=Res;
					USART_RX_STA++;
					if(USART_RX_STA>USART_RX_LEN-1)//数据超出给定长度，重新接收
						USART_RX_STA=0;
				}
			}
		}
	}
}

//void send(int data)
//{
//	USART_SendData(USART1,data);
//	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
//}

void send(u16 a)
{
	USART_SendData(USART1,a);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET);
	USART_RX_STA=0;
}

//串口3
void uart3_init(u32 bound)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB |RCC_APB2Periph_AFIO,ENABLE);//使能GPIO引脚时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);//使能串口时钟
	
	GPIO_InitTypeDef GPIO_InitTypeStruct;//定义GPIO结构体变量
	USART_InitTypeDef USART_InitTypeStruct;//定义串口结构体变量
	NVIC_InitTypeDef NVIC_InitTypeStruct;//定义NVIC结构体变量
	
	GPIO_InitTypeStruct.GPIO_Mode=GPIO_Mode_AF_PP;//初始化GPIO引脚B10及模式
	GPIO_InitTypeStruct.GPIO_Pin=GPIO_Pin_10;
	GPIO_InitTypeStruct.GPIO_Speed =GPIO_Speed_10MHz;
	GPIO_Init(GPIOB,&GPIO_InitTypeStruct);
	
	GPIO_InitTypeStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;//初始化GPIO引脚B11及模式
	GPIO_InitTypeStruct.GPIO_Pin=GPIO_Pin_11;
	GPIO_InitTypeStruct.GPIO_Speed =GPIO_Speed_10MHz;
	GPIO_Init(GPIOB,&GPIO_InitTypeStruct);
	
	USART_InitTypeStruct.USART_BaudRate=bound;//初始化串口USART1
	USART_InitTypeStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitTypeStruct.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
	USART_InitTypeStruct.USART_Parity=USART_Parity_No;
	USART_InitTypeStruct.USART_StopBits=USART_StopBits_1;
	USART_InitTypeStruct.USART_WordLength=USART_WordLength_8b;
	USART_Init(USART3,&USART_InitTypeStruct);
	

	
	NVIC_InitTypeStruct.NVIC_IRQChannel=USART3_IRQn;//初始化中断配置
	NVIC_InitTypeStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitTypeStruct.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitTypeStruct.NVIC_IRQChannelSubPriority=1;
	NVIC_Init(&NVIC_InitTypeStruct);
	
	USART_Cmd(USART3,ENABLE);//使能串口（打开串口）
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);//开启接受中断（接收缓存器非空，则中断开启）
	
}
void USART3_IRQHandler(void)//中断服务函数
{
	uint16_t Res;
	if(USART_GetITStatus(USART3,USART_IT_RXNE)!=RESET)//判断该中断为接收中断
	{
	
		Res=USART_ReceiveData(USART3);//读取接受信息
	//	printf("%d\n",Res);
		if((USART_RX_STA3&0x8000)==0)//未接收到0x0a
		{
			if(USART_RX_STA3&0x4000)//未接收到0x0a，接收到0x0d
			{
				if(Res==10)USART_RX_STA3|=0x8000;//接收数据为0x0a,接收完成
				else USART_RX_STA3=0;//否则数据接收错误，重新接收
			}
			else//未接收到0x0a,未接收到0x0d
			{
				if(Res==13)//接收数据为0x0d
					USART_RX_STA3|=0x4000;
				else //未接收到0x0a,接收数据不为0x0d（在接受数据中）
				{
					USART_RX_BUF3[USART_RX_STA3&0x3FFF]=Res;
					USART_RX_STA3++;
					if(USART_RX_STA3>USART_RX_LEN-1)//数据超出给定长度，重新接收
						USART_RX_STA3=0;
				}
			}
		}
	}
}

void send3(int data)
{
	USART_SendData(USART3,data);
	while(USART_GetFlagStatus(USART3,USART_FLAG_TXE)==RESET);
}

















