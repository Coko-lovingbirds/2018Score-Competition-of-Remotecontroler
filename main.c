/*
 * main.c

 */

#include <stdint.h>
#include <stdbool.h>
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

// PE1 PE2 PE3
uint8_t ReadPin;
uint32_t time_count = 0;
uint32_t time_sec = 0;     //50ms
int8_t strrt[7]={0xEA,0xD2,0xA1,0X02,0x88,0x81,0XED};    //遥杆控制   飞行器停


bool count_flag = true;

void LED_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_1|GPIO_PIN_3);
}


void Timer1_Config(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

	TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
	TimerLoadSet(TIMER1_BASE, TIMER_A, SysCtlClockGet()/100);     //10 ms 记一次

	IntEnable(INT_TIMER1A);
	TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	TimerEnable(TIMER1_BASE, TIMER_A);

}

void NVIC_Configure(void)
{
	IntPrioritySet(INT_TIMER1A,0x00);
    IntPrioritySet(INT_UART1,0x30);
    IntPrioritySet(INT_UART2,0x20);
    IntPrioritySet(INT_ADC0SS0, 0x10);
}

void Timer1IntHandler(void)   //10ms执行一次
{
	TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    //countX++;
	if(count_flag)
	{
		time_count++;
		if(time_count == 5)
		{
			time_count =0;
			time_sec++;
			if(time_sec >= 0xFFFFFFFE)
				time_sec = 0;
		}
	}
	else if(count_flag == false)
	{
		time_sec=0;
	}
}

void key_configure(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));

	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);

	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);

	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x0);

}


void KEY_PF4_Pros(void)
{
	int i;
	ReadPin = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4);
	if((ReadPin&GPIO_PIN_4) != GPIO_PIN_4)
	{
		SysCtlDelay(20*(SysCtlClockGet()/3000));
		ReadPin = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4);
		if((ReadPin&GPIO_PIN_4) != GPIO_PIN_4)
		{
			ADCsend = false;
			for(i=0;i<7;i++)
				UARTCharPutNonBlocking(UART2_BASE,strrt[i]);    //飞行器降落
			ADCsend = true;
			while(!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4))
			{
				;
			}
		}
	}

}


void main(void)
{

    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);

    FPULazyStackingEnable();
    FPUEnable();
    IntMasterEnable();

    LED_Init();
    ConfigureUART0();
    Recive_UART1_Config();
    Recive_UART2_Config();
    Timer1_Config();
    NVIC_Configure();
    ADC_Init();
    key_configure();

    while(1)
    {

    	ADC_Trig();
    	while(true != bDataReady);
    	bDataReady = false;
    	ADC_DataHander();

    	u1Data_handle();
    	Uart0_Command();
 //   	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, ~GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_3));
    	SysCtlDelay(100*(SysCtlClockGet()/3000));
    	KEY_PF4_Pros();

    }
}


