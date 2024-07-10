#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "usart2.h"
#include "dht11.h"
#include "wifi.h"
#include "timer.h"
#include <string.h>

char buf1[100];

char *cmdLEDON = "+MQTTSUBRECV:0,\"dbld_led\",5,LEDON";
char *cmdLEDOFF = "+MQTTSUBRECV:0,\"dbld_led\",6,LEDOFF";

uint32_t tiwen[2];
u8 xl,xy;
int shidu[2],qiwen[2];

void show_all();

int main(void)
{
	delay_init();	    	 //��ʱ������ʼ��	  
	LED_Init();		  	//��ʼ����LED���ӵ�Ӳ���ӿ�
	wifi_GPIO_Init();
	uart_init(9600); 
	USART2_Init(115200); //����ESP8266
	//DHT11_Init();  //dht11
	rst_wifi();
				LED1 = 1;
	init_wifi();
	TIM3_Int_Init(9999,35999); //5s
	while(1)
	{
		
		if(USART2_RX_STA&0X8000)
		{
			strcpy(buf1,(char*)USART2_RX_BUF);
			//printf("���ڶ��յ�:%s\r\n",buf1);
			if(!memcmp(buf1,cmdLEDON,strlen(cmdLEDON)))
			{
				//����
				LED1 = 1;
				printf("����\r\n");
			}
			if(!memcmp(buf1,cmdLEDOFF,strlen(cmdLEDOFF)))
			{
				//�ص�
				LED1 = 0;
				printf("�ص�\r\n");
			}
			
			USART2_RX_STA = 0;
		}
		u8 len ,j;
		
		
		if(USART_RX_STA & 0X8000)
		{
			printf("111\r\n");
			len=USART_RX_STA&0x3fff;//��ȡ�������ݳ���
//	  for(j=0;j<len;j++)//��ѭ��ʵ�ֽ�stm32���յ��������ٷ��͸�����3
//	  {
		  tiwen[0] = USART_RX_BUF[0];
		  tiwen[1] = USART_RX_BUF[1];
		  xl = USART_RX_BUF[2];
		  xy = USART_RX_BUF[3];
		  shidu[0] = USART_RX_BUF[4];
		  shidu[1] = USART_RX_BUF[5];
		  qiwen[0] = USART_RX_BUF[6];
		  qiwen[1] = USART_RX_BUF[7];
//    printf("%d\n",USART_RX_BUF[j]);//USART_RX_BUF[j]�д洢�ľ���С���ɷ�������ֵ�������������ѡ��
//	  }
		  USART_RX_STA = 0;
			
			show_all();
		}
	}
}

void show_all()
{
	OLED_ShowNum(1,7,shidu[0],2);
	OLED_ShowChar(1,9,'.');
	OLED_ShowNum(1,10,shidu[1],2);
	OLED_ShowNum(2,7,qiwen[0]+26,2);
	OLED_ShowChar(2,9,'.');
	OLED_ShowNum(2,10,qiwen[1],2);
	
	OLED_ShowString(3,1,"tiwen:");
	OLED_ShowNum(3,7,tiwen[0],2);
	OLED_ShowString(3,8,".");
	OLED_ShowNum(3,7,tiwen[1],1);
	
	OLED_ShowString(4,1,"xl&xy:");
	OLED_ShowNum(4,7,(uint32_t)xl,3);
	OLED_ShowChar(4,11,'&');
	OLED_ShowNum(4,13,(uint32_t)xy,3);
}