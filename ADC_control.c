/*
 * ADC_control.c
 *
 *  Created on: 2018年7月17日
 *      Author: CZW
 */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
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


//EA SEND_ID ADDRESS_ID 01 00 FF

volatile bool  RunAhead = false;
volatile bool  RunLeft = false;
volatile bool  RunRight = false;
volatile bool  RunRear = false;
volatile bool  RunLeft_rear = false;
volatile bool  RunLeft_ahead = false;
volatile bool  RunRight_rear = false;
volatile bool  RunRight_ahead = false;
volatile bool  ADCsend = false;
//volatile bool ADCflag = false;

volatile bool  bDataReady = true;     // The flag indicates is data ready.
uint32_t X_Ch0Value, Y_Ch1Value,X_Ch2Value;          // Savve the voltage value of ch0 and ch1.



int speedz=0;
int speedy=0;
int speedtempz=(int)((500*1.8)/6);
int speedtempy=(int)((500*1.8)/6);
int distancenow =0;
int distance =0;
int pitchnow=0;
int pitch_ag = 0;

void ADC_Init(void);
void ADC0IntHander(void);
void ADC_Trig(void);

//*****************************************************************************
//
// This function sets up UART0 to be used for a console to display information
// as the example is running.
//
//*****************************************************************************
void ConfigureUART0(void)//串口
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    UARTStdioConfig(0, 9600, 16000000);
}



//
// ADC0 interrupt handler.
//
void ADC0IntHander(void)
{
	uint32_t ui32AdcValue[3] = {0};
    ADCIntClear(ADC0_BASE, 0);
    ADCSequenceDataGet(ADC0_BASE, 0, ui32AdcValue);

    X_Ch0Value = ((float)ui32AdcValue[0])*(3300.0/4096.0);
    Y_Ch1Value = ((float)ui32AdcValue[1])*(3300.0/4096.0);
    X_Ch2Value = ((float)ui32AdcValue[2])*(3300.0/4096.0);
//	UARTprintf("X_Ch0Value:%4umv, X_Ch2Value:%4umv\n", X_Ch0Value,X_Ch2Value);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, ~GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1));
	bDataReady = true;
}


//
// Init the adc.
//
void ADC_Init(void)
{
	//
	// Enable the peripheral adc0.
	//
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0))
			;

	//
	// Init the peripheral GPIOE.
	//
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE))
		;

	//
	// Configure the GPIOE2,3 to peripheral function.
	//
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2 |GPIO_PIN_3|GPIO_PIN_1);

	//
	// Sets the clock configuration for the ADC: use piosc, full rate.
	//
	ADCClockConfigSet(ADC0_BASE, ADC_CLOCK_SRC_PIOSC | ADC_CLOCK_RATE_FULL, 1);
	//
	// Set the reference voltage.
	// Attention: tm4c123x default reference voltage is 3.3v, can't change.
	//
	ADCReferenceSet(ADC0_BASE, ADC_REF_INT);
	//
	// Enable the first sample sequencer to capture the value of channel 0
	// and channel 1 when the processor trigger occurs.
	//
	ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);

	//
	// Configure a step of the sample sequencer.
	//! \param ui32Base is the base address of the ADC module.
	//! \param ui32SequenceNum is the sample sequence number.
	//! \param ui32Step is the step to be configured.
	//! \param ui32Config is the configuration of this step;
	// *Specific information refer to data sheet and drverlib.
	ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH0);
	ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ADC_CTL_CH1);
	ADCSequenceStepConfigure(ADC0_BASE, 0, 2, ADC_CTL_CH2 |ADC_CTL_END | ADC_CTL_IE);
	//
	// Configure the hardware over sample value: 64.
	// param ui32Factor is the number of samples to be averaged.
	//
	ADCHardwareOversampleConfigure(ADC0_BASE, 64);

	//
	// Enables sample sequence0.
	//
	ADCSequenceEnable(ADC0_BASE, 0);

	//
	// Register the interrupt handler.
	//
	ADCIntRegister(ADC0_BASE, 0, ADC0IntHander);

	//
	// Enable the interrupt sequence0 of ADC0.
	//
	ADCIntEnable(ADC0_BASE, 0);

	//
    // set the interrut priority.
	//
    //
    // Enable the interrupt of sequence0.
    //
	IntEnable(INT_SSI0);
}

//
// Trigger a interrupt of ADC0.
//
void ADC_Trig(void)
{
	ADCProcessorTrigger(ADC0_BASE, 0);
}

