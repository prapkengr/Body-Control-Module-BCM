/***************************************************************************
File Name   : CAN_drv_extern.h
Description : CAN bus driver extern definitions
AUTHORS     : 
DATE        : v02
***************************************************************************/

/***********************************
		Macro Definition
***********************************/
#define	CAN_BIT7			((unsigned char)0x80)
#define	CAN_BIT6			((unsigned char)0x40)
#define	CAN_BIT5			((unsigned char)0x20)
#define	CAN_BIT4			((unsigned char)0x10)
#define	CAN_BIT3			((unsigned char)0x08)
#define	CAN_BIT2			((unsigned char)0x04)
#define	CAN_BIT1			((unsigned char)0x02)
#define	CAN_BIT0			((unsigned char)0x01)

#define	CAN_DI()			asm("	and	0xbf,psw	")
#define	CAN_EI()			asm("	or	0x70,psw	")
#define CAN_NOP()			asm("	nop	")

#define	CAN_ON				(1)
#define	CAN_OFF				(0)
#define	CAN_CLR				(0)

#define	CAN_TRUE			((unsigned char)(0x00))
#define	CAN_FALSE			((unsigned char)(0xFF^CAN_TRUE))

#define	CAN_RCV_SLOT_MAX	(4)

#define	CAN_RCVID_ID1		(0x0100)
#define	CAN_RCVID_ID2		(0x0200)
#define	CAN_RCVID_ID3		(0x0300)
#define	CAN_RCVID_ID4		(0x0400)
#define	CAN_RCVID_NOUSE		(0xFFFF)
#define	CAN_RCVDLC_NOUSE	(0xFF)
#define	CAN_INIT_WAIT		(3)
#define	CAN_DATA_TRANS_WAIT	(5)
#define	CAN_RCV_SLOT_NUM	(0x1F)

#define	CAN_PORT_DIR_OUTPUT	(1)
#define	CAN_PORT_DIR_INPUT	(0)
#define	V_CANPMD_SET		(0x01)	/* CTxA/CRxA,CAN Enable */
#define	V_C0CCE_ON			(1)
#define	V_C0CCE_OFF			(0)
#define	V_C0BRPEXT_SET		(0x00)

/* C0SJW = 1, C0BRP = 1, Phase_Seg1 = "2TQ", TQ = 250ns, Baud rate = 250Kbps */
/* Prop_Seg = "10TQ", Phase_Seg2 = "3TQ", (+Sync_Seg = "1TQ") */
/* #define	V_C0BITTMG_L_SET	(0x41)	*/
/* #define	V_C0BITTMG_H_SET	(0x2B)	*/

/* C0SJW = 0, C0BRP = 0, Phase_Seg1 = "1TQ", TQ = 125ns, Baud rate = 1Mbps */
/* Prop_Seg = "4TQ", Phase_Seg2 = "2TQ", (+Sync_Seg = "1TQ") */
#define	V_C0BITTMG_L_SET	(0x00)
#define	V_C0BITTMG_H_SET	(0x14)

#define	CAN_SLOT_32			(0x20)

/******************************************
		Function Prototype Declaration
******************************************/
void vCAN_MainProcessing( void );
void vCAN_InitSetting( void );
void vCAN_PortSetting( void );
void vCAN_PortSetting( void );
void vCAN_BaudrateSetting( void );
void vCAN_ReceiveSlotSetting( void );
void vCAN_SendSlotSetting( void );
void vCAN_IrqSetting( void );
void vCAN_NormalProcessing( void );
void vCAN_Interrupt( void );
void vCAN_BOffIRQ( void );
void vCAN_ReceiveIRQ( void );
void vCAN_SendIRQ( void );
unsigned char ucCAN_SendReq( unsigned char* data_ptr );
