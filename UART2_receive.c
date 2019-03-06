/*
 * UART2_receive.c
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
#include "UART2_receive.h"
#include "UART1_receive.h"

int yaw =0;
bool yaw_updated_flag = false;

void Recive_UART2_Config(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

	HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTD_BASE + GPIO_O_CR) = 0x80;

	GPIOPinConfigure(GPIO_PD6_U2RX);
	GPIOPinConfigure(GPIO_PD7_U2TX);
	GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_6 | GPIO_PIN_7);
	UARTConfigSetExpClk(UART2_BASE,SysCtlClockGet(), 9600,
	                                (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
	                                 UART_CONFIG_PAR_NONE));
    IntPrioritySet(INT_UART2 ,0x11); //b '000'0 0000 //工程中的最高优先级
    IntEnable(INT_UART2);
    UARTIntEnable(UART2_BASE, UART_INT_RX | UART_INT_RT);

}


void UART2IntHandler(void)
{

	uint32_t ui32Status;
//	uint8_t rx_buffer;
//	static bool receive_flag = false;
//	static bool receive_yaw_flag = false;
//	static int index=0;
	ui32Status = UARTIntStatus(UART2_BASE, true);
	UARTIntClear(UART2_BASE, ui32Status);
	/*
	while(UARTCharsAvail(UART2_BASE))
	{
		rx_buffer = (uint8_t)(UARTCharGetNonBlocking(UART2_BASE));
		if(rx_buffer==0xDA)
		{
			receive_flag = true;
			index=0;
		}
		if(receive_flag)
		{
			index += 1;
			if(index==2)
			{
				switch(receive_flag)
				{
				case 0x01:
					receive_yaw_flag=true;
					break;
				}
			}
		}
		if(index==3&&receive_yaw_flag)
		{
			yaw = rx_buffer*255;
		}
		if(index==4&&receive_yaw_flag)
		{
			yaw = yaw + rx_buffer;
			yaw_updated_flag = true;
			receive_flag = false;
			receive_yaw_flag = false;
			index = 0;
		}
	}
	*/
}
