/*
 * UART_receive.c
 *
 *  Created on: 2018年12月17日
 *      Author: CZW
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/fpu.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "ADC_control.h"
#include "inc/hw_types.h"
#include "string.h"
#include "inc/hw_gpio.h"
#include "UART1_receive.h"
#include "UART2_receive.h"

//串口屏通信

int temp1,temp2;
int8_t tempx1,tempx2;
uint32_t timenow,timelast;

char UART1_Rx_Buffers[50];
bool UART1_Updated_Flag=false;
bool flagin1 = false;
bool flagin2 = false;
bool angle_flag = false;
bool str_flag = false;

int8_t str[7]={0xEA,SEND_ID,ADDRESS_ID,0X02,0x00,0x81,0XEF};    //遥感
int8_t data[6]={0xEA,SEND_ID,ADDRESS_ID,0x01,0x00,0xEF};        //摄像头
int8_t run[10]={0xEA,SEND_ID,ADDRESS_ID,0X05,0XFF,0x00,0x00,0xFF,0x81/*速度*/,0xEF};   //屏幕发送
uint32_t numd[8];
uint32_t numa[8];



void Recive_UART1_Config(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0| GPIO_PIN_1);
    UARTConfigSetExpClk(UART1_BASE,SysCtlClockGet(), 9600,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    IntEnable(INT_UART1);
    UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);
}

void UART1IntHandler(void)
{
	uint32_t ui32Status;
	char rx_buffer;
	static bool receive_flag=false;
	int count=0;
	ui32Status = UARTIntStatus(UART1_BASE, true);
	UARTIntClear(UART1_BASE, ui32Status);
	while(UARTCharsAvail(UART1_BASE))
	{
		rx_buffer = (uint8_t)(UARTCharGetNonBlocking(UART1_BASE));
		if(rx_buffer==0xEA)
		{
			receive_flag = true;
			UART1_Updated_Flag =false;
            memset(UART1_Rx_Buffers,0,50);
            count =0;
		}
		if(rx_buffer==0xEF)
		{
			receive_flag = false;
			UART1_Updated_Flag =true;
		}

		if(receive_flag&&rx_buffer!=0xEA)
		{
			UART1_Rx_Buffers[count] = rx_buffer;
			count++;

		}


	}
}



void u1Data_handle(void)
{
	int m=0;
	int n0,n1,res0,res1;

	if(UART1_Updated_Flag==true)
	{
		distance =0;
		pitch_ag =0;
		int i=0;
		int n=1;
		UART1_Updated_Flag = false;
		switch(UART1_Rx_Buffers[0])
		{
			case 0xC9:         //屏控
				ADCsend = false;
				time_sec=0;
				time_count=0;
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2,0x0);
				break;

			case 0xC8: 			//遥控
				ADCsend = true;
				time_sec=0;
				time_count=0;
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2,GPIO_PIN_2);
				break;

			case 0xE1: 			//速度1
				str[5]=0x81;
				run[8]=0x81;
				speedtempz=(int)((350*1.8)/6);
				speedtempy=(int)((350*1.8)/6);

				break;

			case 0xE2: 			//速度2
				str[5]=0x82;
				run[8]=0x82;
				speedtempz=(int)((500*1.8)/6);
				speedtempy=(int)((500*1.8)/6);

				break;

			case 0xE3: 			//速度3
				str[5]=0x83;
				run[8]=0x83;
				speedtempz=(int)((750*1.8)/6);
				speedtempy=(int)((750*1.8)/6);

				break;

			case 0xC0:			//摄像头关
				data[2]=0xA8;
				data[4]=0xC0;
				for(i=0;i<6;i++)
					UARTCharPutNonBlocking(UART2_BASE,data[i]);
				break;

			case 0xC1:			//摄像头开
				data[2]=0xA8;
				data[4]=0xC1;
				for(i=0;i<6;i++)
					UARTCharPutNonBlocking(UART2_BASE,data[i]);
				break;

			case 0xED:                         //直行
				while(UART1_Rx_Buffers[i+1] != 0xCA)
				{
					numd[i] = (UART1_Rx_Buffers[i+1]-48);
					i++;
					flagin1 = true;
				}
				if(flagin1 && (UART1_Rx_Buffers[1] != 0x20))
				{
					flagin1 = false;
					run[7] = UART1_Rx_Buffers[i+2];
					for(;i>0;i--)
					{
						distance += n*numd[i-1];
						n*=10;
					}

					run[4]=0xAA;

					for(m=0;m<5;m++)
						UARTCharPutNonBlocking(UART2_BASE,run[m]);

					res0 = distance % 255;
					n0 = distance / 255;
					UARTCharPutNonBlocking(UART2_BASE,n0);
					UARTCharPutNonBlocking(UART2_BASE,res0);

//					temp1 = distance;

					for(m=7;m<10;m++)
						UARTCharPutNonBlocking(UART2_BASE,run[m]);
//					tempx2 = run[7];
					str_flag = true;
					time_sec=0;
					time_count=0;

				}

				break;

			case 0xEE:                          //转弯
				while(UART1_Rx_Buffers[i+1] != 0xCA)
				{
					numa[i] = (UART1_Rx_Buffers[i+1]-48);
					i++;
					flagin2 = true;
				}
				if(flagin2 && (UART1_Rx_Buffers[1] != 0x20))
				{
					flagin2 = false;
					run[7] = UART1_Rx_Buffers[i+2];
					for(;i>0;i--)
					{
						pitch_ag += n*numa[i-1];
						n*=10;
					}
//					UARTprintf("%d",pitch_ag);

					run[4]=0xBB;

					for(m=0;m<5;m++)
						UARTCharPutNonBlocking(UART2_BASE,run[m]);

					res1 = pitch_ag % 255;
					n1 = pitch_ag / 255;
					UARTCharPutNonBlocking(UART2_BASE,n1);
					UARTCharPutNonBlocking(UART2_BASE,res1);

//					temp2 = pitch_ag;

					for(m=7;m<10;m++)
						UARTCharPutNonBlocking(UART2_BASE,run[m]);

//					tempx1 = run[7];

					angle_flag = true;
					time_count=0;
					time_sec=0;




				}
				break;

		}
        memset(UART1_Rx_Buffers,0,50);

	}
}


void ADC_DataHander(void)
{
	if(ADCsend == true)
	{
		int i;
		if((X_Ch0Value > 1700) && (X_Ch2Value > 1400) && (X_Ch2Value < 1700))
		{
			str[4]=0x11;
			RunAhead = true;
			for(i=0;i<7;i++)
				UARTCharPutNonBlocking(UART2_BASE,str[i]);
			speedz = speedtempz;
			speedy = speedtempy;
			distancenow =(int)(0.05*time_sec*((speedz/60.0)*3.14*3.24*2))/1.2;

		}
		else if((X_Ch0Value < 1500) && (X_Ch2Value > 1400) && (X_Ch2Value < 1700))
		{
			str[4]=0x22;
			RunRear = true;
			for(i=0;i<7;i++)
				UARTCharPutNonBlocking(UART2_BASE,str[i]);
			speedz = speedtempz;
			speedy = speedtempy;
			distancenow =(int)(0.05*time_sec*((speedz/60.0)*3.14*3.24*3.24))/2.0;

		}
		else if((X_Ch0Value > 1500) && (X_Ch0Value < 1700) && (X_Ch2Value > 1700))
		{
			str[4]=0x33;
			RunRight = true;
			for(i=0;i<7;i++)
				UARTCharPutNonBlocking(UART2_BASE,str[i]);
			speedz = speedtempz;
			speedy = speedtempy;

		}
		else if((X_Ch0Value > 1500) && (X_Ch0Value < 1700) && (X_Ch2Value < 1000))
		{
			str[4]=0x44;
			RunLeft = true;
			for(i=0;i<7;i++)
				UARTCharPutNonBlocking(UART2_BASE,str[i]);
			speedz = speedtempz;
			speedy = speedtempy;

		}
		else if((X_Ch0Value > 1500) && (X_Ch0Value < 1700) &&(X_Ch2Value > 1200) && (X_Ch2Value < 1700))//停
		{
			str[4]=0x88;
			for(i=0;i<7;i++)
				UARTCharPutNonBlocking(UART2_BASE,str[i]);
			time_sec=0;
			time_count=0;
			distancenow = 0;
			speedz = 0;
			speedy = 0;
		}

	}

}




void Uart0_Command(void)
{

	if((str_flag == true) && (ADCsend == false))
	{

		speedz=(int)((500*1.8)/6);
		speedy=(int)((500*1.8)/6);
		distancenow =(int)(0.05*time_sec*((speedz/60.0)*3.14*3.24*2));
	}


	if((distancenow >= distance) && ADCsend == false)
	{
		distancenow = distance;
		str_flag = false;
	}

	if((angle_flag == true) && (ADCsend == false))
	{
		if(run[7] == 0x2B)
		{

			speedz=(int)((500*1.8)/6);
			speedy=0;
			pitchnow = 180*((0.05*time_sec*((speedz/60.0)*3.14*3.24*2))/16.5)/3.14;
		}
		else if(run[7] == 0x2D)
		{

			speedy=(int)((500*1.8)/6);
			speedz=0;
			pitchnow = 180*((0.05*time_sec*((speedy/60.0)*3.14*3.24*2))/16.5)/3.14;
		}

	}

	if((pitchnow >= pitch_ag) && ADCsend == false)
	{
		pitchnow = pitch_ag;
		angle_flag = false;
	}

	UARTprintf("n0.val=%d", distancenow);
	UARTprintf("%c%c%c", 0xFF, 0xFF, 0xFF);
	UARTprintf("n1.val=%d", speedz);
	UARTprintf("%c%c%c", 0xFF, 0xFF, 0xFF);
	UARTprintf("n2.val=%d", speedy);
	UARTprintf("%c%c%c", 0xFF, 0xFF, 0xFF);
	UARTprintf("n3.val=%d", pitchnow);
	UARTprintf("%c%c%c", 0xFF, 0xFF, 0xFF);
}

