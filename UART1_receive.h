/*
 * UART1_receive.h
 *
 *  Created on: 2018Äê12ÔÂ17ÈÕ
 *      Author: CZW
 */

#ifndef UART1_RECEIVE_H_
#define UART1_RECEIVE_H_

extern uint32_t time_count;
extern uint32_t time_sec;
extern uint32_t timenow,timelast;


extern char UART1_Rx_Buffers[50];
extern void Recive_UART1_Config(void);
extern void UART1IntHandler(void);
extern void Uart0_Command(void);
extern void u1Data_handle(void);
extern void ADC_DataHander(void);


#endif /* UART1_RECEIVE_H_ */
