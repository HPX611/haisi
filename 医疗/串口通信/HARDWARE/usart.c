#include "usart.h"
#include "sys.h"
#include "stdio.h"


#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

//#if EN_USART_RX//������ڽ���ʹ��

#define USART_RX_LEN 200
uint16_t USART_RX_BUF[USART_REC_LEN];
uint16_t USART_RX_BUF3[USART_REC_LEN];
u16 USART_RX_STA=0;
u16 USART_RX_STA3=0;

void uart_init(u32 bound)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_AFIO,ENABLE);//ʹ��GPIO����ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//ʹ�ܴ���ʱ��
	
	GPIO_InitTypeDef GPIO_InitTypeStruct;//����GPIO�ṹ�����
	USART_InitTypeDef USART_InitTypeStruct;//���崮�ڽṹ�����
	NVIC_InitTypeDef NVIC_InitTypeStruct;//����NVIC�ṹ�����
	
	GPIO_InitTypeStruct.GPIO_Mode=GPIO_Mode_AF_PP;//��ʼ��GPIO����A9��ģʽ
	GPIO_InitTypeStruct.GPIO_Pin=GPIO_Pin_9;
	GPIO_InitTypeStruct.GPIO_Speed =GPIO_Speed_10MHz;
	GPIO_Init(GPIOA,&GPIO_InitTypeStruct);
	
	GPIO_InitTypeStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;//��ʼ��GPIO����A10��ģʽ
	GPIO_InitTypeStruct.GPIO_Pin=GPIO_Pin_10;
	GPIO_InitTypeStruct.GPIO_Speed =GPIO_Speed_10MHz;
	GPIO_Init(GPIOA,&GPIO_InitTypeStruct);
	
	USART_InitTypeStruct.USART_BaudRate=bound;//��ʼ������USART1
	USART_InitTypeStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitTypeStruct.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
	USART_InitTypeStruct.USART_Parity=USART_Parity_No;
	USART_InitTypeStruct.USART_StopBits=USART_StopBits_1;
	USART_InitTypeStruct.USART_WordLength=USART_WordLength_8b;
	USART_Init(USART1,&USART_InitTypeStruct);
	

	
	NVIC_InitTypeStruct.NVIC_IRQChannel=USART1_IRQn;//��ʼ���ж�����
	NVIC_InitTypeStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitTypeStruct.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitTypeStruct.NVIC_IRQChannelSubPriority=1;
	NVIC_Init(&NVIC_InitTypeStruct);
	
	USART_Cmd(USART1,ENABLE);//ʹ�ܴ��ڣ��򿪴��ڣ�
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);//���������жϣ����ջ������ǿգ����жϿ�����
	
}
void USART1_IRQHandler(void)//�жϷ�����
{
	u8 Res;
	if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET)//�жϸ��ж�Ϊ�����ж�
	{
		Res=USART_ReceiveData(USART1);//��ȡ������Ϣ
	//	printf("%d\n",Res);
		if((USART_RX_STA&0x8000)==0)//δ���յ�0x0a
		{
			if(USART_RX_STA&0x4000)//δ���յ�0x0a�����յ�0x0d
			{
				if(Res==0x0a)USART_RX_STA|=0x8000;//��������Ϊ0x0a,�������
				else USART_RX_STA=0;//�������ݽ��մ������½���
			}
			else//δ���յ�0x0a,δ���յ�0x0d
			{
				if(Res==0x0d)//��������Ϊ0x0d
					USART_RX_STA|=0x4000;
				else //δ���յ�0x0a,�������ݲ�Ϊ0x0d���ڽ��������У�
				{
					USART_RX_BUF[USART_RX_STA&0x3FFF]=Res;
					USART_RX_STA++;
					if(USART_RX_STA>USART_RX_LEN-1)//���ݳ����������ȣ����½���
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

//����3
void uart3_init(u32 bound)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB |RCC_APB2Periph_AFIO,ENABLE);//ʹ��GPIO����ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);//ʹ�ܴ���ʱ��
	
	GPIO_InitTypeDef GPIO_InitTypeStruct;//����GPIO�ṹ�����
	USART_InitTypeDef USART_InitTypeStruct;//���崮�ڽṹ�����
	NVIC_InitTypeDef NVIC_InitTypeStruct;//����NVIC�ṹ�����
	
	GPIO_InitTypeStruct.GPIO_Mode=GPIO_Mode_AF_PP;//��ʼ��GPIO����B10��ģʽ
	GPIO_InitTypeStruct.GPIO_Pin=GPIO_Pin_10;
	GPIO_InitTypeStruct.GPIO_Speed =GPIO_Speed_10MHz;
	GPIO_Init(GPIOB,&GPIO_InitTypeStruct);
	
	GPIO_InitTypeStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;//��ʼ��GPIO����B11��ģʽ
	GPIO_InitTypeStruct.GPIO_Pin=GPIO_Pin_11;
	GPIO_InitTypeStruct.GPIO_Speed =GPIO_Speed_10MHz;
	GPIO_Init(GPIOB,&GPIO_InitTypeStruct);
	
	USART_InitTypeStruct.USART_BaudRate=bound;//��ʼ������USART1
	USART_InitTypeStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitTypeStruct.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
	USART_InitTypeStruct.USART_Parity=USART_Parity_No;
	USART_InitTypeStruct.USART_StopBits=USART_StopBits_1;
	USART_InitTypeStruct.USART_WordLength=USART_WordLength_8b;
	USART_Init(USART3,&USART_InitTypeStruct);
	

	
	NVIC_InitTypeStruct.NVIC_IRQChannel=USART3_IRQn;//��ʼ���ж�����
	NVIC_InitTypeStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitTypeStruct.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitTypeStruct.NVIC_IRQChannelSubPriority=1;
	NVIC_Init(&NVIC_InitTypeStruct);
	
	USART_Cmd(USART3,ENABLE);//ʹ�ܴ��ڣ��򿪴��ڣ�
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);//���������жϣ����ջ������ǿգ����жϿ�����
	
}
void USART3_IRQHandler(void)//�жϷ�����
{
	uint16_t Res;
	if(USART_GetITStatus(USART3,USART_IT_RXNE)!=RESET)//�жϸ��ж�Ϊ�����ж�
	{
	
		Res=USART_ReceiveData(USART3);//��ȡ������Ϣ
	//	printf("%d\n",Res);
		if((USART_RX_STA3&0x8000)==0)//δ���յ�0x0a
		{
			if(USART_RX_STA3&0x4000)//δ���յ�0x0a�����յ�0x0d
			{
				if(Res==10)USART_RX_STA3|=0x8000;//��������Ϊ0x0a,�������
				else USART_RX_STA3=0;//�������ݽ��մ������½���
			}
			else//δ���յ�0x0a,δ���յ�0x0d
			{
				if(Res==13)//��������Ϊ0x0d
					USART_RX_STA3|=0x4000;
				else //δ���յ�0x0a,�������ݲ�Ϊ0x0d���ڽ��������У�
				{
					USART_RX_BUF3[USART_RX_STA3&0x3FFF]=Res;
					USART_RX_STA3++;
					if(USART_RX_STA3>USART_RX_LEN-1)//���ݳ����������ȣ����½���
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

















