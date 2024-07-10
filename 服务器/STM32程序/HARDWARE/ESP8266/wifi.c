#include "stm32f10x.h"                  // Device header
#include "wifi.h"
#include "delay.h"
#include "usart.h"
#include "usart2.h"
#include "string.h"
#include "led.h"

/*
			ESP01s       STM32
			 3V3----------3.3V
			 GND----------GND
			 RX-----------PA2
			 TX-----------PA3
			 RST----------PA4
*/

//��һ����wifiģ���ϵ�������һ��

void wifi_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;                     
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA , ENABLE);  
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;                
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;        
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   		 
	GPIO_Init(GPIOA, &GPIO_InitStructure);            		 
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
}

void rst_wifi(void)
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	delay_ms(1000);
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
}


//�ڶ�������ʼ����ATָ������


//�жϴ��ڶ��յ��������ǲ���ǰ�涨���ack���ڴ���Ӧ������
u8* wifi_check_cmd(u8 *str)
{
	char *strx = 0;
	if(USART2_RX_STA&0X8000)
	{
		USART2_RX_BUF[USART2_RX_STA&0X7FFF] = 0;
		strx = strstr((const char*)USART2_RX_BUF,(const char*)str);
	}
	return (u8*)strx;
}


//��һ�����������
//cmd�����͵�ATָ��
//ack���ڴ��Ļش�
//time���ȴ�ʱ��(��λ10ms)
//����ֵ��0�����ͳɹ��� 1������ʧ��
u8 wifi_send_cmd(u8 *cmd,u8 *ack,u16 time)
{
	u8 res = 0;
	USART2_RX_STA = 0;
	
	u2_printf("%s\r\n",cmd);
	if(time)
	{
		while(--time)
		{
			delay_ms(10);
			if(USART2_RX_STA&0X8000) //���ڶ����յ�����
			{
				//�жϽ��ܵ������ǲ�����Ҫ��
				if(wifi_check_cmd(ack))
				{
					break;
				}
				USART2_RX_STA = 0;
			}
		}
		if(time == 0) res = 1;
	}
	return res;
	
}

//����������˳����ATָ��
void init_wifi(void)
{
	//1 AT
	while(wifi_send_cmd("AT","OK",50))
	{
		LED0=1;
		printf("AT��Ӧʧ��\r\n");
	}
		LED0=0;
	
	//2 ��Wi-Fiģ������ΪStation��STA��ģʽ
	while(wifi_send_cmd("AT+CWMODE=1","OK",50))
	{
		printf("STAģʽ����ʧ��\r\n");
	}
	
	//3 ����WIFI���û���������
//	while(wifi_send_cmd("AT+CWJAP=\"xiaomi\",\"123456789\"","OK",500))
	while(wifi_send_cmd("AT+CWJAP=\"HY...\",\"hyhyhhh12345\"","OK",500))
	{
		printf("����WIFIʧ��\r\n");
	}
	
	//4 ����MQTT�������
	while(wifi_send_cmd("AT+MQTTUSERCFG=0,1,\"MQTTID\",\"dbldhwd\",\"dbldhwd\",0,0,\"\"","OK",500))
	{
		printf("����WIFIʧ��\r\n");
	}
  //5 ����MQTT��ip
	while(wifi_send_cmd("AT+MQTTCONN=0,\"broker-cn.emqx.io\",1883,1","OK",500))
	{
		printf("����MQTT������ʧ��\r\n");
	}	
	//6 ��������
	while(wifi_send_cmd("AT+MQTTSUB=0,\"dbld_led\",0","OK",50))
	{
		printf("��������ʧ��\r\n");
	}	
}



