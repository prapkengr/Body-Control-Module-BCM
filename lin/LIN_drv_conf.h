/**********************************************************************************
#ifdef DOC
FILE NAME: LIN_drv_conf.h
DESCRIPTION: LIN info table header file
REMARK:
DATE:
#endif
**********************************************************************************/
/***********************************
		Macro Definition
***********************************/
#define	LIN_RES_VALUE		(2)											/* Depend on the number of nodes in the system	*/

/**********************************
Macro Definition
**********************************/
#define	LIN_RX_PORT			(P0IN&0x04)									/* Depend on the number of nodes in the system	*/

/**********************************
Time related definition
**********************************/
#define	LIN_OTIME_THRESHOLD			(800)								/*   Frame over time threshold definition */
#define	LIN_RSOTIME_THRESHOLD		(5)									/*  Response space overtime threshold definition */
#define	LIN_INTER_BYTE_SPACE_CNT	(2)									/*  Transmission interval definition between bytes */
#define	LIN_RESPONSE_SPACE_CNT		(2)									/*  Response transmission wait time definition */
#define	LIN_TIMER_SPACE_UNIT		(19)

