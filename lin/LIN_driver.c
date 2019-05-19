/**********************************************************************************
#ifdef DOC
FILE NAME	: LIN_driver.c
DESCRIPTION : LIN function source file
REMARK		:
DATE		:
#endif
**********************************************************************************/

/**********************************
		Include File
**********************************/
#include "sys_type.h"
#define	LIN_DRV_H
#include	"LIN_driver.h"
#include	"MCU.h"
#include	"LIN_drv_conf.h"
#undef	LIN_DRV_H


/**********************************
		Macro Definition
**********************************/
unsigned char		ucLIN_ControlFlag;									/* 	*/
#define	LIN_RESPONSE_END_ACK_SET()	(ucLIN_ControlFlag|=0x01)			/* 	*/
#define	LIN_RESPONSE_END_ACK_CLR()	(ucLIN_ControlFlag&=0xFE)			/* 	*/
#define	LIN_RESPONSE_END_ACK_REF()	(ucLIN_ControlFlag&0x01)			/* 	*/
#define	LIN_RESPONSE_ERR_ACK_SET()	(ucLIN_ControlFlag|=0x02)			/* 	*/
#define	LIN_RESPONSE_ERR_ACK_CLR()	(ucLIN_ControlFlag&=0xFD)			/* 	*/
#define	LIN_RESPONSE_ERR_ACK_REF()	(ucLIN_ControlFlag&0x02)			/* 	*/
#define	LIN_RESPONSE_TX_RUNNING_SET()	(ucLIN_ControlFlag|=0x04)		/* 	*/
#define	LIN_RESPONSE_TX_RUNNING_CLR()	(ucLIN_ControlFlag&=0xFB)		/* 	*/
#define	LIN_RESPONSE_TX_RUNNING_REF()	(ucLIN_ControlFlag&0x04)		/* 	*/

LIN_RESPONSE_INFO	LIN_ResponsInfoTable[LIN_RES_VALUE] = {
	/* {"ID","data length","usage","checksum type"} */
	{0x10,0x01,LIN_RESINFO_USAGE_TX,LIN_RESINFO_CSUM_CLASSIC},
	{0x20,0x02,LIN_RESINFO_USAGE_RX,LIN_RESINFO_CSUM_CLASSIC},
};

/**********************************
		Variable Declaration
**********************************/
extern unsigned char	ucBRD_MODE;										/* 	*/
unsigned char		ucLIN_FrameMode;									/* 	*/
unsigned char		ucLIN_Sc0str;										/* 	*/
unsigned char		ucLIN_RxBuf;										/* 	*/
unsigned char		ucLIN_RxPointer;									/* 	*/
unsigned char		ucLIN_RxProtectID;									/* 	*/
unsigned char		ucLIN_RxData[8];									/* 	*/
unsigned char		ucLIN_RxChecksum;									/* 	*/
unsigned short		usLIN_CalcCheckSum;									/* 	*/
unsigned char		ucLIN_ReceiveDataCount;								/* 	*/
unsigned short		usLIN_OverTimeCounter;								/* 	*/
unsigned char		ucLIN_TxData[8];									/* 	*/
unsigned char		ucLIN_TxBuf;										/* 	*/
unsigned char		ucLIN_TxWaitCounter;								/* 	*/
LIN_COM_DATA		LIN_ComData[LIN_RES_VALUE];							/* 	*/
LIN_COM_DATA_BUF	LIN_ComDataBuf[LIN_RES_VALUE][2];					/* 	*/
LIN_COM_DATA_TX		LIN_ComDataTx[LIN_RES_VALUE][2];					/* 	*/
unsigned char		ucLIN_ComDataBufP[LIN_RES_VALUE];					/* 	*/
unsigned char		ucLIN_ComDataTxP[LIN_RES_VALUE];					/* 	*/

/******************************************
		Function Prototype Declaration
******************************************/
void vLIN_Initialize(void);											/* LIN initialization						*/
void vLIN_UARTStart(void);												/* Set UART ports and interrupt enable		*/
void vLIN_UARTStop(void);												/* Disable UART ports and interrupt enable	*/
void vLIN_UARTInterrupt(void);											/* UART interrupt service routine			*/
void vLIN_ErrorIRQ(void);												/* UART ISR error case						*/
void vLIN_ReceiveIRQ(void);												/* UART ISR normal receive case				*/
void vLIN_TimerIRQ(void);												/* LIN overtime ISR							*/
void vLIN_FrameEndProcessing(void);										/* Frame end processing						*/
void vLIN_Update_Rx_ComDataBuf(unsigned char,unsigned char*);			/* LIN RX buffer update processing			*/
void vLIN_Update_Tx_ComDataBuf(unsigned char,unsigned char);			/* LIN TX buffer update processing			*/
void vLIN_MainProcessing(void);											/* LIN main function					*/
void vLIN_Update_ComData(void);											/* Update of LIN data buffer interface to app layer	*/

/******************************************
		Interrupt function pragmas
******************************************/
#pragma _interrupt	vLIN_UARTInterruptTx
#pragma _interrupt	vLIN_UARTInterruptRx
#pragma _interrupt	vLIN_TimerIRQ


/******************************************************************
#ifdef DOC
FUNCTION	: vLIN_Initialize
DESCRIPTION	: LIN initialize function
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
******************************************************************/
void vLIN_Initialize( void )
{
	unsigned char	loop;

	/* Initialize data buffer pointer	*/
	for( loop = 0; loop < LIN_RES_VALUE; loop ++ )
	{
		ucLIN_ComDataBufP[loop] = 1;
		ucLIN_ComDataTxP[loop] = 1;
	}

	/* Initialize Timer 2 (for inter-byte overtime interrupt)	*/
	TM2MD &= 0xF7;							/* Count Stop */

	CK2MD = 0x02;							/* fpll-div/16 */
	TM2MD = 0x01;							/* Clk source = TM2PSC output, Normal Timer operation */
	TM2OC = LIN_TIMER_SPACE_UNIT;

	/* Initialize port special function for LIN	*/
	P0PLUD &= 0xF3;							/* P02,P03 internal PU unused */
	P0ODC &= 0xF7;							/* P03 push-pull output */
	P0DIR &= 0xFB;							/* P02 RXD0A */
	P0OUT |= 0x08;
	P0DIR |= 0x08;							/* P03 TXD0A */

	/* Initialize Timer A as Serial 0 clock source */
	TMAMD1 = 0x00;							/* Timer A Mode Register 1 */
											/* 		BIT0-2 ---- 000: fpll-div */
											/* 		BIT3 ------ 0: stop operation */
											/* 		BIT4-7 ---- 0 */
	
	TMAMD2 = 0x00;							/* Timer A Mode Register 2 */
											/* 		BIT0-5 ---- 0 */
											/* 		BIT6 ------ Prescaler operation disabled */
											/* 		BIT7 ------ 0 */

	TMAOC = 103;							/* Timer A Compare Register */

	TMAMD2 |= 0x40;							/* Prescaler operation enabled */
	TMAMD1 |= 0x08;							/* Enable Timer operation */

	/* Serial 0 -> UART settings initialization */
	SC0SEL = 0x07;							/* Serial 0 I/O Pin Switching Control Register */
											/* 		BIT0-2 ---- 111: Timer A selected */
											/* 		BIT3 ------ 0 */
											/* 		BIT4-6 ---- 000: Timer output */
											/* 		BIT7 ------ 0: UART output */

	SC0MD3 = 0x06;							/* Serial Interface 0 Mode Register 3 */
											/* 		BIT0-2 ---- 11X: Clock selection (Timer output) */
											/* 		BIT3 ------ 0: Prescaler count disabled */
											/* 		BIT4-5 ---- 0 */
											/* 		BIT6-7 ---- 00: SBO state after final data Tx = HIGH */

	SC0MD0 = 0x18;							/* Serial 0 Mode Register 0 */
											/* 		BIT0-2 ---- 000: Don't care, only apply to sync serial */
											/* 		BIT3 ------ 1: Start condition enabled */
											/* 		BIT4 ------ 1: LSB sent first */
											/* 		BIT5-6 ---- 0 */
											/* 		BIT7 ------ 0: Don't care, only apply to sync serial */

	SC0MD2 = 0x88;							/* Serial 0 Mode Register 2 */
											/* 		BIT0 ------ 0: Default to data transmission */
											/* 		BIT1 ------ 0: Default to data reception */
											/* 		BIT2 ------ 0 */
											/* 		BIT3 ------ 1: Disable parity bit */
											/* 		BIT4-5 ---- 00: Don't care(?) */
											/* 		BIT6-7 ---- 10: 8 data bits + 1 stop bit */

	SC0MD1 = 0x0D;							/* Serial 0 Mode Register 1 */
											/* 		BIT0 ------ 1: Duplex UART operation */
											/* 		BIT1 ------ 0: Transfer clock div by 8 */
											/* 		BIT2 ------ 1: Clock master */
											/* 		BIT3 ------ 1: Transfer clock divided */
											/* 		BIT4 ------ 0: SBO unused (as normal port) */
											/* 		BIT5 ------ 0: SBI unused (as fixed '1' input) */
											/* 		BIT6 ------ 0: SBT unused (as normal port) */
											/* 		BIT7 ------ 0: Data input from SBI/RXD pin */

	/* IRQ initialization */
	asm("	and	0xbf,psw	");				/* MIE = 0 */
	MEMCTR |= 0x04;							/* IRWE = 1: Enable SW write to xxxIR */

	TM2ICR  = 0x80;							/* IRQ Disable, Clear IR bit, TM2LV = '10' */

	SC0RICR = 0x40;							/* Serial 0 UART Reception Interrupt Control Register */
											/* 		BIT0 ------ 0: Clear interrupt request */
											/* 		BIT1 ------ 0: Interrupt disabled */
											/* 		BIT6-7 ---- 01: Mask level '01' */

	SC0TICR = 0x40;							/* Serial 0 Interrupt Control Register */
											/* 		BIT0 ------ 0: Clear interrupt request */
											/* 		BIT1 ------ 0: Interrupt disabled */
											/* 		BIT6-7 ---- 01: Mask level '01' */

	LINICR  = 0x60;							/* LIN Interrupt Control Register */
											/* 		BIT0 ------ 0: Clear interrupt request */
											/* 		BIT1 ------ 0: Interrupt disabled */
											/* 		BIT5 ------ 1: Interrupt posedge detect */
											/* 		BIT6-7 ---- 01: Mask level '01' */
	
	MEMCTR &= 0xFB;							/* IRWE = 0: Disable SW write to xxxIR */
	asm("	or 0x70,psw		");				/* IM = "11", MIE = '1' */

	if( ucBRD_MODE ) {
		for( loop = 0; loop < LIN_RES_VALUE; loop ++ ) {
			if( LIN_ResponsInfoTable[loop].info_flag.usage == LIN_RESINFO_USAGE_TX ) {
				LIN_ResponsInfoTable[loop].info_flag.usage = LIN_RESINFO_USAGE_RX;
			}
			else {
				LIN_ResponsInfoTable[loop].info_flag.usage = LIN_RESINFO_USAGE_TX;
			}
		}
	}
}

/******************************************************************
#ifdef DOC
FUNCTION	: vLIN_UARTStart
DESCRIPTION	: Set UART ports and interrupt enable
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
******************************************************************/
void vLIN_UARTStart( void )
{
	SC0MD1  &= 0xCF;						/* Disable SBO/SBI and use as normal ports */
	TMAMD1  |= 0x08;						/* Start Timer A operation */
	SC0RICR |= 0x02;						/* Set interrupt enable */
	SC0TICR |= 0x02;						/* Set interrupt enable */
	SC0MD1  |= 0x30;						/* Set ports to function as UART ports */
}

/******************************************************************
#ifdef DOC
FUNCTION	: vLIN_UARTStop
DESCRIPTION	: Disable UART ports and interrupt enable
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
******************************************************************/
void vLIN_UARTStop( void )
{
	TMAMD1  &= 0xF7;						/* Stop Timer A operation */
	SC0MD1  &= 0xCF;						/* Disable SBO/SBI and use as normal ports */
	SC0RICR &= 0xFD;						/* Disable UART Rx interrupt */
	SC0TICR &= 0xFD;						/* Disable UART Tx interrupt */
}


/******************************************************************
#ifdef DOC
FUNCTION	: vLIN_UARTInterruptRx
DESCRIPTION	: UART receive interrupt service routine
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
******************************************************************/
void vLIN_UARTInterruptRx( void )
{
	ucLIN_Sc0str = SC0STR;					/* Read Serial 0 Status Register 		*/
	ucLIN_RxBuf = RXBUF0;					/* Read contents of receive data		*/

	if( (ucLIN_Sc0str&0x01) == 0x01 )		/* Check for error case					*/
	{
		vLIN_ErrorIRQ();					/* Implement serial reception error interrupt processing					*/
	}
	else									/*  error monitor flag = no error 				*/
	{
		vLIN_ReceiveIRQ();					/* Execution of serial reception interrupt processing	*/
	}
}

/******************************************************************
#ifdef DOC
FUNCTION	: vLIN_ErrorIRQ
DESCRIPTION	: UART receive ISR error case
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
******************************************************************/
void vLIN_ErrorIRQ( void )
{
	if( ucLIN_FrameMode == LIN_FMD_FRAME_ERR )			/* Frame control mode "Wait for framing error" 	*/
	{
		if( (ucLIN_Sc0str&0x08) == 0x08 )				/* With framing error 		*/
		{
			if( ucLIN_RxBuf == 0x00 )					/* All received data "Low"	*/
			{
				vLIN_UARTStop();						/* UART reception stop	*/

				usLIN_OverTimeCounter = 0x0000;			/* Frame over time count start	*/

				ucLIN_RxProtectID = 0;					/* ID											*/
				ucLIN_RxData[0] = 0;					/* Data1										*/
				ucLIN_RxData[1] = 0;					/* Data2										*/
				ucLIN_RxData[2] = 0;					/* Data3										*/
				ucLIN_RxData[3] = 0;					/* Data4										*/
				ucLIN_RxData[4] = 0;					/* Data5										*/
				ucLIN_RxData[5] = 0;					/* Data6										*/
				ucLIN_RxData[6] = 0;					/* Data7										*/
				ucLIN_RxData[7] = 0;					/* Data8										*/
				ucLIN_RxChecksum = 0;					/* Checksum										*/

				ucLIN_FrameMode = LIN_FMD_BREAK_END;	/* Transition to frame control mode "Wait for break completion" */
			}
		}
	}
}

/******************************************************************
#ifdef DOC
FUNCTION	: vLIN_ReceiveIRQ
DESCRIPTION	: UART receive ISR normal case
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
******************************************************************/
void vLIN_ReceiveIRQ( void )
{

}

/******************************************************************
#ifdef DOC
FUNCTION	: vLIN_TimerIRQ
DESCRIPTION	: LIN inter-byte/response timeout ISR
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
******************************************************************/
void vLIN_TimerIRQ( void )
{

}

/******************************************************************
#ifdef DOC
FUNCTION	: vLIN_FrameEndProcessing
DESCRIPTION	: LIN Frame end processing
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
******************************************************************/
void vLIN_FrameEndProcessing( void )
{

}

/******************************************************************
#ifdef DOC
FUNCTION	: vLIN_Update_Rx_ComDataBuf
DESCRIPTION	: LIN RX buffer update processing
INPUT		: unsigned char res_no, unsigned char* data_pnt
OUTPUT		: void
RETURN		: void
#endif
******************************************************************/
void vLIN_Update_Rx_ComDataBuf( unsigned char res_no, unsigned char* data_pnt )
{

}

/******************************************************************
#ifdef DOC
FUNCTION	: vLIN_Update_Tx_ComDataBuf
DESCRIPTION	: LIN TX buffer update processing
INPUT		: unsigned char res_no, unsigned char status
OUTPUT		: void
RETURN		: void
#endif
******************************************************************/
void vLIN_Update_Tx_ComDataBuf( unsigned char res_no, unsigned char status )
{

}

/******************************************************************
#ifdef DOC
FUNCTION	: vLIN_MainProcessing
DESCRIPTION	: LIN main function
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
******************************************************************/
void vLIN_MainProcessing( void )
{

}

/******************************************************************
#ifdef DOC
FUNCTION	: vLIN_Update_ComData
DESCRIPTION	: Update of LIN data buffer interface to app layer
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
******************************************************************/
void vLIN_Update_ComData( void )
{

}

/******************************************************************
#ifdef DOC
FUNCTION	: vLIN_StartTxHdr
DESCRIPTION	: Create LIN header frame (for master mode)
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
******************************************************************/
void vLIN_StartTxHdr( void )
{


}


/******************************************************************
#ifdef DOC
FUNCTION	: vLIN_UARTInterruptTx
DESCRIPTION	: UART transmit interrupt service routine
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
******************************************************************/
void vLIN_UARTInterruptTx( void )
{
	TM2ICR |= 0x02;							/* IRQ Enable */
	TM2MD  |= 0x08;							/* Count Start */

}

/******************************************************************
#ifdef DOC
FUNCTION	: vLIN_Interrupt
DESCRIPTION	: Slave mode header frame field reception handling
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
******************************************************************/
void vLIN_Interrupt( void )
{


}

