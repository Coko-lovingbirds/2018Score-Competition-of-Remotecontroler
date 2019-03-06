/*
 * UART2_receive.h
 *
 *  Created on: 2018Äê12ÔÂ17ÈÕ
 *      Author: CZW
 */

#ifndef UART2_RECEIVE_H_
#define UART2_RECEIVE_H_

extern int yaw;
extern bool yaw_updated_flag;

extern void Recive_UART2_Config(void);
extern void UART2IntHandler(void);

#endif /* UART2_RECEIVE_H_ */
