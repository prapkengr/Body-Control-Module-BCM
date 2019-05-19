/***************************************************************************
File Name   : CAN_driver.c
Description :
AUTHORS     : 
DATE        : Date , v02
***************************************************************************/
/**********************************
		Include File
**********************************/
#include	"MCU.h"
#include	"CAN_drv_extern.h"


/**********************************
		Macro Definition
**********************************/
#define DEBUG_MODE				(0)
#define DEBUG_SILENT			(0x08)
#define DEBUG_LBACK				(0x10)

#define	CAN_OPMODE_INIT			(0x00)
#define	CAN_OPMODE_BOFF_WAIT	(0x01)
#define	CAN_OPMODE_NORMAL		(0x02)

#define	CAN_BOFF_WAIT_LIMIT		(10)

/**********************************
		Variable Declaration
**********************************/
extern unsigned short	usCAN_TxID;
static unsigned char	ucCAN_OperationMode;
static unsigned char	ucCAN_SendDataBuffer[12];

unsigned char			ucCAN_NowRcvData[11];
unsigned char 			ucINT_Proc_RxData;
unsigned char			ucCAN_NowSendID[2]={0x00,0x00};

extern const unsigned short	usCAN_RcvIDTable[31] = {
	CAN_RCVID_ID1,		/* ID"x100" reception permission */
	CAN_RCVID_ID2,		/* ID"x200" reception permission */
	CAN_RCVID_ID3,		/* ID"x300"  reception permission */
	CAN_RCVID_ID4,		/* ID"x400" reception permission */
	CAN_RCVID_NOUSE 	/* Receive slot not used */
};

/**********************************
**********************************/

/**********************************
ŠO•”�éŒ¾
**********************************/


/******************************************************************
FUNCTION	: vCAN_MainProcessing( void )
DESCRIPTION	: 
REMARK		: 
INPUT		: 
OUTPUT		: 
******************************************************************/
void vCAN_MainProcessing( void )
{
	static unsigned char	boff_wait = 0x00;

	switch( ucCAN_OperationMode ){

		default:
		case CAN_OPMODE_INIT:
			vCAN_InitSetting();
			boff_wait = (unsigned char)CAN_CLR;
			ucCAN_OperationMode = CAN_OPMODE_BOFF_WAIT;
			break;

		case CAN_OPMODE_BOFF_WAIT:
			if( (C0STAT&CAN_BIT7) == CAN_BIT7 )
			{
				boff_wait ++;
				if( boff_wait >= CAN_BOFF_WAIT_LIMIT )
				{
					ucCAN_OperationMode = CAN_OPMODE_INIT;
				}
			}
			else
			{
				ucCAN_OperationMode = CAN_OPMODE_NORMAL;
			}
			break;

		case CAN_OPMODE_NORMAL:
			if( (C0STAT&CAN_BIT7) == CAN_BIT7 )
			{
				ucCAN_OperationMode = CAN_OPMODE_INIT;
			}
			else
			{
				vCAN_NormalProcessing();
			}
			break;
	}
}

/******************************************************************
FUNCTION	: vCAN_InitSetting
DESCRIPTION	: 
REMARK		: 
INPUT		: 
OUTPUT		: 
******************************************************************/
void vCAN_InitSetting( void )
{
	unsigned char	lp_wait;

	/* CAN initialization */
	lp_wait = (unsigned char)CAN_CLR;
	do{
		C0CANCTR |= CAN_BIT0;				/* Set C0Init bit */
		lp_wait ++;
	}while( ((C0CANCTR&CAN_BIT0) != CAN_BIT0) && (lp_wait < CAN_INIT_WAIT) );	/* Wait until C0Init is set */

	C0IFCNTRL |= CAN_BIT2;					/* CAN0 controller clock supply enable */

	vCAN_PortSetting();

	vCAN_BaudrateSetting();

	vCAN_ReceiveSlotSetting();

	vCAN_SendSlotSetting();

	vCAN_IrqSetting();

	/* CAN transition to NORMAL operation */
	lp_wait = (unsigned char)CAN_CLR;
	do{
		C0CANCTR &= (0xFF^CAN_BIT0);		/* Clear C0Init bit */
		lp_wait ++;
	}while( ((C0CANCTR&CAN_BIT0) == CAN_BIT0) && (lp_wait < CAN_INIT_WAIT) );	/* Wait until C0Init is cleared */

	/* Enable CAN interrupt */
	CANICR |= CAN_BIT1;
	CAN_EI();								/* Already set in MAIN */
}

/******************************************************************
FUNCTION	: vCAN_PortSetting
DESCRIPTION	: 
REMARK		: 
INPUT		: 
OUTPUT		: 
******************************************************************/
void vCAN_PortSetting( void )
{
	/* Tx/Rx port settings */
	P5DIR &= (0xFF^CAN_BIT4);				/* P54=CRxA */
	P5DIR |= CAN_BIT3;						/* P53=CTxA */
	CANPMD = V_CANPMD_SET;					/* Select CTxA/CRxA, enable CAN serial in/out*/
}

/******************************************************************
FUNCTION	: vCAN_BaudrateSetting
DESCRIPTION	: 
REMARK		: 
INPUT		: 
OUTPUT		: 
******************************************************************/
void vCAN_BaudrateSetting( void )
{
	C0CANCTR |= CAN_BIT6;					/* Set C0CCE to enable writing to Bit Timing reg */
	C0BRPEXT = V_C0BRPEXT_SET;				/* Baud rate prescaler extension */
	C0BITTMG_L = V_C0BITTMG_L_SET;			/* Bit Timing register L */
	C0BITTMG_H = V_C0BITTMG_H_SET;			/* Bit Timing register H */
	C0CANCTR &= (0xFF^CAN_BIT6);			/* Clear C0CCE to disable writing to Bit Timing reg */

	#if DEBUG_MODE
	{
		C0CANCTR |= CAN_BIT7;				/* Set Test Mode during DEBUG MODE */
		C0CANTST |= (unsigned char)(DEBUG_SILENT | DEBUG_LBACK);
	}
	#endif
}

/******************************************************************
FUNCTION	: vCAN_ReceiveSlotSetting
DESCRIPTION	: 
REMARK		: 
INPUT		: 
OUTPUT		: 
******************************************************************/
void vCAN_ReceiveSlotSetting( void )
{
	unsigned char	lp_cnt;
	unsigned char	lp_wait;

	for( lp_cnt = (unsigned char)CAN_CLR; lp_cnt < CAN_RCV_SLOT_NUM; lp_cnt ++ )
	{

		if( usCAN_RcvIDTable[lp_cnt] == CAN_RCVID_NOUSE )
		{
			C0IF1CMDMSK = 0xA0;	/* Write mode, enable ARB register access only */
			C0IF1ARB2_H = 0x00;	/* Write all 0 ID (for unused buffers) */
		}
		else
		{
			C0IF1CMDMSK   = 0xF3;			/* Write mode (C0IF1 -> Msg Buffer) */
											/* All C0IF1 register access enabled */
			C0IF1MSK2_H   = 0x5F;			/* Standard ID Mask, 11bits [28:18] */
			C0IF1MSK2_L   = 0xFC;
			C0IF1MSK1_H   = 0x00;
			C0IF1MSK1_L   = 0x00;
			C0IF1ARB2_H   = ((((unsigned char)(usCAN_RcvIDTable[lp_cnt] >> 6)) & 0x1F) | 0x80);
													/* Set Msg Buffer enable, Read Msg direction and 11bit ID */
			C0IF1ARB2_L   = ((((unsigned char)usCAN_RcvIDTable[lp_cnt]) << 2) & 0xFC);
			C0IF1ARB1_H   = 0x00;
			C0IF1ARB1_L   = 0x00;
			C0IF1MSGCNT_L = 0x88;			/* Single message buffer, DLC = 8byte */
			C0IF1MSGCNT_H = 0x14;			/* ID mask enabled, Reception INT enabled */
			C0IF1DATA1_L  = 0x00;			/* Initialize all data to 0 */
			C0IF1DATA1_H  = 0x00;
			C0IF1DATA2_L  = 0x00;
			C0IF1DATA2_H  = 0x00;
			C0IF1DATB1_L  = 0x00;
			C0IF1DATB1_H  = 0x00;
			C0IF1DATB2_L  = 0x00;
			C0IF1DATB2_H  = 0x00;

		}

		C0IF1CMDRQ_L = (lp_cnt + 1);		/* Message Buffer number = 33 */
		for( lp_wait=(unsigned char)CAN_CLR ;
			((C0IF1CMDRQ_H&CAN_BIT7)==CAN_BIT7) && (lp_wait<CAN_DATA_TRANS_WAIT) ;lp_wait++ )
		{
			/* Wait for data to be transferred from C0IF1 registers to Message Buffers */
		}
	}
}

/******************************************************************
FUNCTION	: vCAN_SendSlotSetting
DESCRIPTION	: 
REMARK		: 
INPUT		: 
OUTPUT		: 
******************************************************************/
void vCAN_SendSlotSetting( void )
{
	unsigned char	lp_wait;

	C0IF2CMDMSK   = 0xB0;					/* Write access to ARB and MSGCNT  */
	C0IF2ARB2_H   = ((((unsigned char)(usCAN_TxID >> 6)) & 0x1F) | 0xA0);
											/* Enable Msg Buffer, Std frame ID, Tx direction */
	C0IF2ARB2_L   = ((((unsigned char)usCAN_TxID) << 2) & 0xFC);
	C0IF2ARB1_H   = 0x00;
	C0IF2ARB1_L   = 0x00;

	C0IF2MSGCNT_L = 0x88;					/* Single message buffer, DLC = 8byte */
	C0IF2MSGCNT_H = 0x08;					/* ID mask disabled, Transmission INT enabled */

	C0IF2CMDRQ_L = CAN_SLOT_32;				/* Message Buffer number = 32 */
	for( lp_wait=(unsigned char)CAN_CLR ;
		((C0IF2CMDRQ_H&CAN_BIT7)==CAN_BIT7) && (lp_wait<CAN_DATA_TRANS_WAIT) ;lp_wait++ )
	{
		/* Wait for data to be transferred from C0IF1 registers to Message Buffers */
	}
}

/******************************************************************
FUNCTION	: vCAN_IrqSetting
DESCRIPTION	: 
REMARK		: 
INPUT		: 
OUTPUT		: 
******************************************************************/
void vCAN_IrqSetting( void )
{
	unsigned char	lp_wait;
	unsigned char	lp_cnt;

	ucINT_Proc_RxData = 0;

	CANICR 	  &= (0xFF^CAN_BIT1);			/* CAN interrupt disable during setting */
	CANICR 	  &= (0xFF^CAN_BIT7);			/* Set CAN interrupt to LVL = "01" */
	CANICR 	  |=  CAN_BIT6;
		
	C0CANCTR  |=  CAN_BIT3;					/* Error status interrupt enable */
	C0CANCTR  &= (0xFF^CAN_BIT2);			/* Status change interrupt disabled */
	C0CANCTR  |=  CAN_BIT1;					/* Notification enable of interrupt */

	C0IFCNTRL &= (0xFF^CAN_BIT1);			/* Disable return from stop status after a wake-up int */
	C0IFCNTRL &= (0xFF^CAN_BIT0);			/* Wake-up interrupt disable */

	/* Message Buffer interrupt pending flag clear */
	C0IF1CMDMSK = 0x08;
	for( lp_cnt = (unsigned char)CAN_CLR; lp_cnt <= 0x1F; lp_cnt ++ )
	{
		C0IF1CMDRQ_L = (lp_cnt + 1);		/* perform read to clear all IntPnd bit */
		for( lp_wait=(unsigned char)CAN_CLR ;
			((C0IF1CMDRQ_H&CAN_BIT7)==CAN_BIT7) && (lp_wait<CAN_DATA_TRANS_WAIT) ;lp_wait++ )
		{
			/* Wait for data to be transferred from Message Buffers to C0IF1 registers */
		}
	}
}

/******************************************************************
FUNCTION	: vCAN_NormalProcessing
DESCRIPTION	: 
REMARK		: 
INPUT		: 
OUTPUT		: 
******************************************************************/
void vCAN_NormalProcessing( void )
{
	unsigned char	lp_wait;

	if( ucCAN_SendDataBuffer[0] == 0x01 )
	{
		if( (C0TXRQ2_H&CAN_BIT7) != CAN_BIT7 )
		{
			/* ID */
			C0IF2ARB2_H = (0xA0|((ucCAN_SendDataBuffer[2]<<2)&0x1C)|((ucCAN_SendDataBuffer[1]>>6)&0x03));
			C0IF2ARB2_L = ((ucCAN_SendDataBuffer[1]<<2)&0xFC);

			/* DLC */
			C0IF2MSGCNT_L = 0x80 | (ucCAN_SendDataBuffer[3] & 0x0F);

			/* Data */
			C0IF2DATA1_L = ucCAN_SendDataBuffer[4];
			C0IF2DATA1_H = ucCAN_SendDataBuffer[5];
			C0IF2DATA2_L = ucCAN_SendDataBuffer[6];
			C0IF2DATA2_H = ucCAN_SendDataBuffer[7];
			C0IF2DATB1_L = ucCAN_SendDataBuffer[8];
			C0IF2DATB1_H = ucCAN_SendDataBuffer[9];
			C0IF2DATB2_L = ucCAN_SendDataBuffer[10];
			C0IF2DATB2_H = ucCAN_SendDataBuffer[11];

			/* Tx-Request */
			C0IF2MSGCNT_H = 0x09;

			/* ƒf�[ƒ^“]‘— */
			C0IF2CMDMSK = 0xB7;
			C0IF2CMDRQ_L = CAN_SLOT_32;
			for( lp_wait=(unsigned char)CAN_CLR ;
				((C0IF2CMDRQ_H&CAN_BIT7)==CAN_BIT7) && (lp_wait<CAN_DATA_TRANS_WAIT) ;lp_wait++ )
			{
				/* ‹ó�ˆ—�wait */
			}

			ucCAN_SendDataBuffer[0] = 0x00;
		}
	}
}

/******************************************************************
FUNCTION	: vCAN_Interrupt
DESCRIPTION	: 
REMARK		: 
INPUT		: 
OUTPUT		: 
******************************************************************/
#pragma _interrupt	vCAN_Interrupt
void vCAN_Interrupt( void )
{
	while( (C0CANINT_H != (unsigned char)CAN_CLR) || (C0CANINT_L != (unsigned char)CAN_CLR) )
	{
		if( (C0CANINT_H == 0x80) && (C0CANINT_L == 0x00) )	/* Status interrupt occurred */
		{
			if( (C0STAT&CAN_BIT7) == CAN_BIT7 )	/* Bus off occurred */
			{
				vCAN_BOffIRQ();
			}
		}
		else if( C0CANINT_H == 0x00 )
		{
			if( (C0CANINT_L >= 0x01) && (C0CANINT_L <= 0x1F) )	/* Message reception interrupt occurred */
			{
				vCAN_ReceiveIRQ();	
			}
			else if( C0CANINT_L == 0x20 )	/* Message transmission interrupt occurred */
			{
				vCAN_SendIRQ();
			}
		}
	}
}


/******************************************************************
FUNCTION	: vCAN_BOffIRQ
DESCRIPTION	: 
REMARK		: 
INPUT		: 
OUTPUT		: 
******************************************************************/
void vCAN_BOffIRQ( void )
{

}

/******************************************************************
FUNCTION	: vCAN_ReceiveIRQ
DESCRIPTION	: 
REMARK		: 
INPUT		: 
OUTPUT		: 
******************************************************************/
void vCAN_ReceiveIRQ( void )
{
	unsigned char	lp_wait;
	unsigned char	m_work;

	m_work = C0CANINT_L;

	C0IF1CMDMSK = 0x3F;
	C0IF1CMDRQ_L = m_work;
	for( lp_wait=(unsigned char)CAN_CLR ;
		((C0IF1CMDRQ_H&CAN_BIT7)==CAN_BIT7) && (lp_wait<CAN_DATA_TRANS_WAIT) ;lp_wait++ )
	{

	}

	ucCAN_NowRcvData[0] = (((C0IF1ARB2_H<<6)&0xC0)|((C0IF1ARB2_L>>2)&0x3F));	/* ID lower 8 bits */
	ucCAN_NowRcvData[1] = ((C0IF1ARB2_H>>2)&0x07);								/* ID upper 3bit */
	ucCAN_NowRcvData[2] = C0IF1MSGCNT_L & 0x0F;		/* DLC */
	ucCAN_NowRcvData[3] = C0IF1DATA1_L;				/* DATA1 */
	ucCAN_NowRcvData[4] = C0IF1DATA1_H;				/* DATA2 */
	ucCAN_NowRcvData[5] = C0IF1DATA2_L;				/* DATA3 */
	ucCAN_NowRcvData[6] = C0IF1DATA2_H;				/* DATA4 */
	ucCAN_NowRcvData[7] = C0IF1DATB1_L;				/* DATA5 */
	ucCAN_NowRcvData[8] = C0IF1DATB1_H;				/* DATA6 */
	ucCAN_NowRcvData[9] = C0IF1DATB2_L;				/* DATA7 */
	ucCAN_NowRcvData[10] = C0IF1DATB2_H;			/* DATA8 */

	C0STAT &= (0xFF^CAN_BIT4);

	ucINT_Proc_RxData = 1;
}

/******************************************************************
FUNCTION	: vCAN_SendIRQ
DESCRIPTION	: 
REMARK		: 
INPUT		: 
OUTPUT		: 
******************************************************************/
void vCAN_SendIRQ( void )
{
	unsigned char	lp_wait;
	unsigned char	send_id_low,send_id_high;

	send_id_low = 0xFF;
	send_id_high = 0xFF;

	if( ((C0INTPND2_H&CAN_BIT7) == CAN_BIT7) &&
		((C0TXRQ2_H&CAN_BIT7) != CAN_BIT7) && ((C0NEWDAT2_H&CAN_BIT7) != CAN_BIT7) )	/* Data transmission complete */
	{
		C0IF1CMDMSK = 0x28;	/* ID read / IndPnd flag clear */
		C0IF1CMDRQ_L = CAN_SLOT_32;
		for( lp_wait=(unsigned char)CAN_CLR ;
			((C0IF1CMDRQ_H&CAN_BIT7)==CAN_BIT7) && (lp_wait<CAN_DATA_TRANS_WAIT) ;lp_wait++ )
		{
			/* Empty processing wait */
		}

		send_id_low = (C0IF1ARB2_L & 0xFC);
		send_id_high = (C0IF1ARB2_H & 0x1F);
	}

	ucCAN_NowSendID[0] = send_id_low;
	ucCAN_NowSendID[1] = send_id_high;

	C0STAT &= (0xFF^CAN_BIT3);

	/* Describe application processing */
}

/******************************************************************
FUNCTION	: ucCAN_SendReq
DESCRIPTION	: 
REMARK		: 
INPUT		: 
OUTPUT		: 
******************************************************************/
unsigned char ucCAN_SendReq( unsigned char* data_ptr )
{
	unsigned char	result;

	result = 0xFF;

	if( ucCAN_SendDataBuffer[0] == 0x00 )
	{

		ucCAN_SendDataBuffer[1] = data_ptr[0];	/* ID-L */
		ucCAN_SendDataBuffer[2] = data_ptr[1];	/* ID-H */
		ucCAN_SendDataBuffer[3] = data_ptr[2];	/* DLC */
		ucCAN_SendDataBuffer[4] = data_ptr[3];	/* DATA1 */
		ucCAN_SendDataBuffer[5] = data_ptr[4];	/* DATA2 */
		ucCAN_SendDataBuffer[6] = data_ptr[5];	/* DATA3 */
		ucCAN_SendDataBuffer[7] = data_ptr[6];	/* DATA4 */
		ucCAN_SendDataBuffer[8] = data_ptr[7];	/* DATA5 */
		ucCAN_SendDataBuffer[9] = data_ptr[8];	/* DATA6 */
		ucCAN_SendDataBuffer[10] = data_ptr[9];	/* DATA7 */
		ucCAN_SendDataBuffer[11] = data_ptr[10];	/* DATA8 */

		ucCAN_SendDataBuffer[0] = 0x01;	/* Tx-Request */
		result = 0x00;
	}

	return( result );
}

