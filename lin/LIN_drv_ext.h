/******************************************************************
File Name: LIN_drv_ext.h
Description: LIN driver external declaration header
AUTHORS:
DATE:
******************************************************************/

/**********************************
Variable external declaration
**********************************/

/**********************************
Function external declaration
**********************************/
extern void vLIN_Initialize(void);
extern void vLIN_UARTInterrupt(void);
extern void vLIN_TimerIRQ(void);
extern void vLIN_MainProcessing(void);

