#include "stm32f10x.h"
#include "led.h"
#include "sys.h"
#include "delay.h"

//������Ҫ���
#include "usart.h"
#include "ds18b20.h"
#include "MAX30102.h"
#include "MAX30102_IIC.h"
#include "FMQ.h"
#include "algorithm.h"
#include "dht11.h"
#include "OLED.h"
#include "usart2.h"
#include "wifi.h"
#include "timer.h"
#include <string.h>
char buf1[100];

char *cmdLEDON = "+MQTTSUBRECV:0,\"dbld_led\",5,LEDON";
char *cmdLEDOFF = "+MQTTSUBRECV:0,\"dbld_led\",6,LEDOFF";



//���ڱ���
uint16_t op = 0;
u16 len,j;

extern u16 USART_RX_STA3;
extern uint16_t USART_RX_BUF3[USART_REC_LEN]; 
extern int rec_data[4];

//���ʱ���
#define MAX_BRIGHTNESS 255
#define INTERRUPT_REG 0X00
u8 value[3];//�������ֵ
uint32_t aun_ir_buffer[500]; //IR LED����������
int32_t n_ir_buffer_length;    //���ݳ���
uint32_t aun_red_buffer[500];    //��ɫLED����������
int32_t n_sp02; //SPO2ֵ
int8_t ch_spo2_valid;   //������ʾSP02�����Ƿ���Ч��ָʾ��
int32_t n_heart_rate;   //����ֵ
int8_t  ch_hr_valid;    //������ʾ���ʼ����Ƿ���Ч��ָʾ��
uint8_t Temp;
uint32_t un_min, un_max, un_prev_data;  
int i;
int32_t n_brightness;
float f_temp;
u8 temp[6];
u8 str[100];
u8 dis_hr=0,dis_spo2=0; 
uint32_t tiwen;


short get_tem(void);
void get_heart(void);
void show_all();



int main(void)
{
	delay_init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	//ͨ�ţ����ڣ���ʼ��
	uart_init(9600);
	uart3_init(9600);
	
	printf("11111\r\n");
	
	//����ģ���ʼ��
	mfq_Init();
	max30102_init();
	IIC_Init();
	
	OLED_Init();
	//�¶ȳ�ʼ��
	DS18B20_Init();
	OLED_ShowString(1,1,"shidu:");						//?
	OLED_ShowString(2,1,"wendu:");
	
	LED_Init();		  	//��ʼ����LED���ӵ�Ӳ���ӿ�
//	wifi_GPIO_Init();
//	USART2_Init(115200); //����ESP8266
//	rst_wifi();
//	//LED1 = 1;
//	init_wifi();
//	TIM3_Int_Init(9999,35999); //5s
//	OLED_ShowString(3,1,"wendu:");
	
	while(1)
	{
		delay_ms(1000);
		
		show_all();
		 //??????????
		
		OLED_ShowNum(1,7,rec_data[2],2);
		OLED_ShowChar(1,9,'.');
		OLED_ShowNum(1,10,rec_data[3],2);
		OLED_ShowNum(2,7,rec_data[0]+26,2);
		OLED_ShowChar(2,9,'.');
		OLED_ShowNum(2,10,rec_data[1],2);

		
//		if(USART2_RX_STA&0X8000)
//		{
//			strcpy(buf1,(char*)USART2_RX_BUF);
//			//printf("���ڶ��յ�:%s\r\n",buf1);
//			if(!memcmp(buf1,cmdLEDON,strlen(cmdLEDON)))
//			{
//				//����
//				LED1 = 1;
//				printf("����\r\n");
//			}
//			if(!memcmp(buf1,cmdLEDOFF,strlen(cmdLEDOFF)))
//			{
//				//�ص�
//				LED1 = 0;
//				printf("�ص�\r\n");
//			}
//			
//			USART2_RX_STA = 0;
//		}
  	}
	
}

void show_all()
{
	OLED_ShowString(3,1,"tiwen:");
	tiwen=(uint32_t)get_tem();
	
	DHT11_REC_Data();
	
	OLED_ShowNum(3,7,tiwen/10,2);
	OLED_ShowString(3,9,".");
	OLED_ShowNum(3,10,tiwen%10,1);
	OLED_ShowString(4,1,"xl&xy:");
	get_heart();
	
	if(tiwen > 360)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_13); 
		delay_ms(1000);
		GPIO_ResetBits(GPIOB,GPIO_Pin_13); 
	}
	
	send(tiwen/10);
	send(tiwen%10);
	send(value[0]);
	send(value[1]);
	send(rec_data[2]/10);
	send(rec_data[3]);
	send(rec_data[0]+26);
	send(rec_data[1]);
//	printf("%d %d %d %d %d %d %d %d",tiwen/10,tiwen%10,value[0],value[1],rec_data[2],rec_data[3],rec_data[0]+26,rec_data[1]);
	send(0x0d);
	send(0x0a);
	OLED_ShowNum(4,7,(uint32_t)value[0],3);
	OLED_ShowChar(4,11,'&');
	OLED_ShowNum(4,13,(uint32_t)value[1],3);
}

//ͨ��
//�ú�������С���ɷ����������ݣ�����1,2����ע��С���ɷ�������Ҫ��c a��β���Ѿ����������Ҫ��
uint16_t get_op(void)
{
	while(1)
	{
		
		if(((USART_RX_STA&0x8000)>>15))//�ж�stm32�Ƿ���ܵ�С��������
		{
//		  len=USART_RX_STA&0x3fff;//��ȡ�������ݳ���
//			for(j=0;j<len;j++)//��ѭ��ʵ�ֽ�stm32���յ��������ٷ��͸�����3
//			{
//				op = USART_RX_BUF[j];//�õ�ѡ��
//				printf("%d\n",USART_RX_BUF[j]);//USART_RX_BUF[j]�д洢�ľ���С���ɷ�������ֵ�������������ѡ��
//			}
  		USART_RX_STA=0;//��ս��ձ�־λ
			
			return USART_RX_BUF[j];
	  }
		
	}
}

//�¶�
//�ú���ֱ�ӷ��ز�õ��¶�ֵ������36.7�򷵻�367��
short get_tem(void)
{
	 short tempture = 0;

	 tempture = DS18B20_Get_Temp();
	 tempture += 37;
		  
	 return tempture;
}

//����
//�ú���ִ�к�,����value[0],value[1]�зֱ������ʡ�Ѫ��ֵ������ֱ��ʹ��
void get_heart(void)
{
	 
	 un_min=0x3FFFF;
	 un_max=0;
	 n_ir_buffer_length=500; //����������Ϊ100���ɴ洢��100sps���е�5������
	
	//��ȡǰ500����������ȷ���źŷ�Χ
	 for(i=0;i<n_ir_buffer_length;i++)
	 {
			while(MAX30102_INT==1);   //�ȴ���ֱ���ж����Ŷ���
			
			max30102_FIFO_ReadBytes(REG_FIFO_DATA,temp);
			aun_red_buffer[i] =  (long)((long)((long)temp[0]&0x03)<<16) | (long)temp[1]<<8 | (long)temp[2];    // Combine values to get the actual number
			aun_ir_buffer[i] = (long)((long)((long)temp[3] & 0x03)<<16) |(long)temp[4]<<8 | (long)temp[5];   // Combine values to get the actual number
					
			if(un_min>aun_red_buffer[i])
					un_min=aun_red_buffer[i];    //�����ź���Сֵ
			if(un_max<aun_red_buffer[i])
					un_max=aun_red_buffer[i];    //�����ź����ֵ
 	}
	un_prev_data=aun_red_buffer[i];
	
	//����ǰ500��������ǰ5���������������ʺ�Ѫ�����Ͷ�
	maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid); 
	
	//ѭ��д���ˣ�������Ҫ��
	//while(1)
//	{
		  for(i=100;i<500;i++)
			{
					aun_red_buffer[i-100]=aun_red_buffer[i];
					aun_ir_buffer[i-100]=aun_ir_buffer[i];
					
					//update the signal min and max
					if(un_min>aun_red_buffer[i])
					un_min=aun_red_buffer[i];
					if(un_max<aun_red_buffer[i])
					un_max=aun_red_buffer[i];
			}
			
			for(i=400;i<500;i++)
			{
					un_prev_data=aun_red_buffer[i-1];
					while(MAX30102_INT==1);
					max30102_FIFO_ReadBytes(REG_FIFO_DATA,temp);
					aun_red_buffer[i] =  (long)((long)((long)temp[0]&0x03)<<16) | (long)temp[1]<<8 | (long)temp[2];    // Combine values to get the actual number
					aun_ir_buffer[i] = (long)((long)((long)temp[3] & 0x03)<<16) |(long)temp[4]<<8 | (long)temp[5];   // Combine values to get the actual number
			
					if(aun_red_buffer[i]>un_prev_data)
					{
							f_temp=aun_red_buffer[i]-un_prev_data;
							f_temp/=(un_max-un_min);
							f_temp*=MAX_BRIGHTNESS;
							n_brightness-=(int)f_temp;
							if(n_brightness<0)
									n_brightness=0;
					}
					else
					{
							f_temp=un_prev_data-aun_red_buffer[i];
							f_temp/=(un_max-un_min);
							f_temp*=MAX_BRIGHTNESS;
							n_brightness+=(int)f_temp;
							if(n_brightness>MAX_BRIGHTNESS)
									n_brightness=MAX_BRIGHTNESS;
					}
					//ͨ��UART�������ͼ��������͵��ն˳���
					if(ch_hr_valid == 1 && n_heart_rate<120)//**/ ch_hr_valid == 1 && ch_spo2_valid ==1 && n_heart_rate<120 && n_sp02<101
					{
							dis_hr = n_heart_rate;
							dis_spo2 = n_sp02;
					}
					else
					{
							dis_hr = 0;
							dis_spo2 = 0;
					}
					value[0] = dis_hr;//��������
					value[1] = dis_spo2;//����Ѫ��
		// }
		 maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);
	   
	}
} 