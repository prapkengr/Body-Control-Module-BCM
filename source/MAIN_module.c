/**********************************************************************************
#ifdef DOC
FILE NAME	: MAIN_module.c
DESCRIPTION     : Main source file
REMARK		:
DATE		: 
#endif
**********************************************************************************/

/**********************************
		Include File
**********************************/
#include "../header/MCU.h"
#include "sys_type.h"
#include "motor.h"
#include "adc.h"
#include "CAN_drv_extern.h"
#include "LIN_driver.h"
#include "LIN_drv_conf.h"
#include "LIN_drv_ext.h"

/**********************************
		Macro Definition
**********************************/
#define	USE_EXTERNAL_OSC (0)
#define	MAIN_TIMER_1MS	(125)
#define	MAIN_LOOP_COUNT	MAIN_TIMER_1MS
#define	MOTOR_LOOP_MS		(50)
#define	SENSOR_LOOP_MS		(12)

/* SW1_IN: To be replaced by P62 input */
#define BRD_MODE		(1)		/* 0:master, 1:slave */
/* SW2_IN: To be replaced by P63 input */
#define SYS_BUS			(1)		/* 0:LIN, 1:CAN */

/*
#define SW3_P67			(0x80)
#define SW3_P66			(0x40)
#define SW3_IN			(UCHAR)(((P6IN & SW3_P67)|(P6IN & SW3_P66)) >> 6)*/
						/* value :  master :  slave  */
						/*   0   :   LED   : HallIC  */
						/*   1   :  Buzzer :  ADCIn  */
						/*   2   :   LCD   :  PWMIn  */
						/*   3   :   RSV   :   RSV   */
/* To be replaced by P67 P66 input */
#define SW3_IN			(1)
						
						
/**********************************
		Variable Declaration
**********************************/
static UCHAR	ucMAIN_LoopStartCount;
UCHAR			ucMAIN_can_send_data[5];
FRAME0			MOTOR_FRAME0;

UCHAR			ucBRD_MODE;
UCHAR			ucSYS_BUS;
USHORT			usCAN_TxID;
USHORT			usCAN_DestID;

extern	LIN_COM_DATA	LIN_ComData[LIN_RES_VALUE];
extern	UCHAR   ucMOTOR_state;
extern	UCHAR	ucINT_Proc_RxData;
extern	UCHAR	ucCAN_NowRcvData[11];
extern	LIN_RESPONSE_INFO	LIN_ResponsInfoTable[LIN_RES_VALUE];

const UCHAR LED_7SEG[] = { 	0xFC, /* 0 */
							0x60, /* 1 */
							0xDA, /* 2 */
							0xF2, /* 3 */
							0x66, /* 4 */
							0xB6, /* 5 */
							0xBE, /* 6 */
							0xE4, /* 7 */
							0xFE, /* 8 */
							0xF6  /* 9 */ };

/******************************************************************
#ifdef DOC
FUNCTION	: void main(void)
DESCRIPTION	: main function
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
******************************************************************/
void main( void )
{
  UCHAR ucMAIN_Motor_loop       =  0;
  UCHAR ucMAIN_Sensor_loop      =  0;
  UCHAR ucMAIN_RcvData_DevID    = 0;
  UCHAR ucMAIN_RcvData_MotorDir = 0;

  vMAIN_Initialize();
  vMAIN_StartMainCounter();

  vMAIN_Disp7Seg( 0x00 );

  while(1)
  {

  if( ucBRD_MODE )
  {
	vADC_MainProcessing();						/* Slave mode function */

	if( ucINT_Proc_RxData )						/* Received a frame */
	{
		ucMAIN_RcvData_MotorDir = (UCHAR)(0x03 & (ucCAN_NowRcvData[3] >> 4));
		switch( ucMAIN_RcvData_MotorDir )
		{
			case 1 :
			{
				vMOTOR_Start(MOTOR_FWD);
				break;
			}
			case 2 :
			{
				vMOTOR_Start(MOTOR_RVS);
				break;
			}
			default : 
			{
				vMOTOR_Idle();
				break;
			}
		}
		ucINT_Proc_RxData = 0;
	}
  }
  else
  {
	vMOTOR_MasterMain();						/* Master mode function */

	if( ucMAIN_Motor_loop >= MOTOR_LOOP_MS )
	{
		MOTOR_FRAME0.MOTOR.MotID = MOTOR_DevID;
		MOTOR_FRAME0.MOTOR.MDir  = ucMOTOR_state;
		MOTOR_FRAME0.MOTOR.Spd   = 0;
		MOTOR_FRAME0.MOTOR.Rst   = 0;

		if( ucSYS_BUS ){
			vMAIN_CANTxFrame( usCAN_DestID, 0x01, MOTOR_FRAME0.BYTE, 0x00 );
		}
		else{
			vMAIN_LINTxFrame( LIN_MOTOR_ID, 0x01, MOTOR_FRAME0.BYTE, 0x00 );
		}

		ucMAIN_Motor_loop = 0;
	}
	else{
		ucMAIN_Motor_loop++;
	}

	/* Set Frame header request to poll target slave and get response frame */
	if( ucMAIN_Sensor_loop >= SENSOR_LOOP_MS )
	{
		if( !ucSYS_BUS ){
			vMAIN_LINTxFrame( LIN_SENSOR_ID, 0x01, 0x00, 0x00 );
		}

		ucMAIN_Sensor_loop = 0;
	}
	else{
		ucMAIN_Sensor_loop++;
	}

    vLIN_MainProcessing();

	if( ucINT_Proc_RxData )						/* Received a frame */
	{
		ucMAIN_RcvData_DevID    = (UCHAR)(0x0F & ucCAN_NowRcvData[3]);
		switch( ucMAIN_RcvData_DevID )			/* Input mode */
		{
			case 0 :
			{
				break;
			}
			case 1 :
			{
				vMAIN_Disp7Seg( ucCAN_NowRcvData[4] );
				break;
			}
			case 2 :
			{
				break;
			}
			default : break;
		}
		ucINT_Proc_RxData = 0;
	}
  } /* if (BRD_MODE) */

	vCAN_MainProcessing();						/* Both */
    vMAIN_WaitMainCounter();					/* Both */
  } /* while (1) */
}

/******************************************************************
#ifdef DOC
FUNCTION	: void vMAIN_Initialize(void)
DESCRIPTION	: Initialization for IOs, timer and clock settings
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
******************************************************************/
void vMAIN_Initialize( void )
{
	vMAIN_InitializeIO();
	vMAIN_SystemClockSetting();
	vMAIN_GetSWInputs();
	vMAIN_Init7Seg();
	vMOTOR_Init();
	vADC_Init();
	vLIN_Initialize();
	vMAIN_StartMainTimer();
}

/*******************************************************************
#ifdef DOC
FUNCTION	: void vMAIN_InitializeIO(void)
DESCRIPTION	: Initialization for IOs
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
*******************************************************************/
void vMAIN_InitializeIO( void )
{

/********************/
/* Port0		*/
/********************/
	P0OUT = 0x00;
	P0DIR = 0x00;
	/* P07 : [-/-] -					*/
	/* P06 : [-/-] -					*/
	/* P05 : [I/-] N.C.					*/
	/* P04 : [I/-] N.C.					*/
	/* P03 : [I/-] N.C.					*/
	/* P02 : [I/-] N.C.					*/
	/* P01 : [I/-] N.C.					*/
	/* P00 : [I/-] N.C.					*/

/********************/
/* Port2		*/
/********************/
#if	USE_EXTERNAL_OSC
	P2OUT = 0x80;
	P2DIR = 0x40;
	/* P27 : [I/-] NRST					*/
	/* P26 : [O/-] OSC2					*/
	/* P25 : [I/-] OSC1					*/
	/* P24 : [I/-] N.C.					*/
	/* P23 : [I/-] N.C.					*/
	/* P22 : [I/-] N.C.					*/
	/* P21 : [I/-] N.C.					*/
	/* P20 : [I/-] N.C.					*/
#else	/* else of USE_EXTERNAL_OSC */
	P2OUT = 0x80;
	P2DIR = 0x00;
	/* P27 : [I/-] NRST					*/
	/* P26 : [I/-] N.C.					*/
	/* P25 : [I/-] N.C.					*/
	/* P24 : [I/-] N.C.					*/
	/* P23 : [I/-] N.C.					*/
	/* P22 : [I/-] N.C.					*/
	/* P21 : [I/-] N.C.					*/
	/* P20 : [I/-] N.C.					*/
#endif	/* end of USE_EXTERNAL_OSC */

/********************/
/* Port5		*/
/********************/
	P5OUT = 0x00;
	P5DIR = 0x00;
	/* P57 : [I/-] N.C.					*/
	/* P56 : [I/-] N.C.					*/
	/* P55 : [I/-] N.C.					*/
	/* P54 : [I/-] N.C.					*/
	/* P53 : [I/-] N.C.					*/
	/* P52 : [I/-] N.C.					*/
	/* P51 : [I/-] N.C.					*/
	/* P50 : [I/-] N.C.					*/

/********************/
/* Port6		*/
/********************/
	P6OUT = 0x00;
	P6DIR = 0x00;
	/* P67 : [I/-] N.C.					*/
	/* P66 : [I/-] N.C.					*/
	/* P65 : [I/-] N.C.					*/
	/* P64 : [I/-] N.C.					*/
	/* P63 : [I/-] N.C.					*/
	/* P62 : [I/-] N.C.					*/
	/* P61 : [-/-] -					*/
	/* P60 : [-/-] -					*/
	
/********************/
/* Port7		*/
/********************/
	P7OUT = 0x00;
	P7DIR = 0x00;
	/* P77 : [I/-] N.C.					*/
	/* P76 : [I/-] N.C.					*/
	/* P75 : [I/-] N.C.					*/
	/* P74 : [I/-] N.C.					*/
	/* P73 : [I/-] N.C.					*/
	/* P72 : [I/-] N.C.					*/
	/* P71 : [I/-] N.C.					*/
	/* P70 : [I/-] N.C.					*/

/********************/
/* Port8		*/
/********************/
	P8OUT = 0x00;
	P8DIR = 0x00;
	/* P87 : [I/-] N.C.					*/
	/* P86 : [I/-] N.C.					*/
	/* P85 : [I/-] N.C.					*/
	/* P84 : [I/-] N.C.					*/
	/* P83 : [I/-] N.C.					*/
	/* P82 : [I/-] N.C.					*/
	/* P81 : [I/-] N.C.					*/
	/* P80 : [I/-] N.C.					*/

/********************/
/* Port9		*/
/********************/
	P9OUT = 0x00;
	P9DIR = 0x00;
	/* P97 : [-/-] -					*/
	/* P96 : [-/-] -					*/
	/* P95 : [-/-] -					*/
	/* P94 : [I/-] N.C.					*/
	/* P93 : [-/-] -					*/
	/* P92 : [-/-] -					*/
	/* P91 : [I/-] N.C.					*/
	/* P90 : [I/-] N.C.					*/
	
/********************/
/* PortA		*/
/********************/
	PAOUT = 0x00;
	PADIR = 0x00;
	/* PA7 : [I/-] N.C.					*/
	/* PA6 : [I/-] N.C.					*/
	/* PA5 : [I/-] N.C.					*/
	/* PA4 : [I/-] N.C.					*/
	/* PA3 : [I/-] N.C.					*/
	/* PA2 : [I/-] N.C.					*/
	/* PA1 : [I/-] N.C.					*/
	/* PA0 : [I/-] N.C.					*/
}

/******************************************************************
#ifdef DOC
FUNCTION	: vMAIN_SystemClockSetting
DESCRIPTION	: Clock settings
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
******************************************************************/
void vMAIN_SystemClockSetting( void )
{
#if USE_EXTERNAL_OSC
	USHORT i,j;
#endif	/* end of USE_EXTERNAL_OSC */

	HANDSHAKE = 0x04;							/* Internal ROM Access Method Control register; set HANDSHAKE method */

#if USE_EXTERNAL_OSC
	OSCCNT |= BP_BIT0;							/* Set P25/P26 as High Speed Oscillation pin, HOSCCNT = 1 */

	for(i = 0; i <= 250;  i++){					/* Wait for oscillation stabilization wait time after setting HOSCCNT flag to 1*/
		asm("	nop		;\n");
	}

	OSCCNT |= BP_BIT1;							/* Set External High Speed Oscillation, HOSCSEL = 1 */

	OSCCNT &= (UCHAR)(0xFF^(BP_BIT4 | BP_BIT5 | BP_BIT6 | BP_BIT7));	/* Set HOSCDIV STOP to active, HOSCDIV SEL to "no dividing" */
	
	CHDRVSEL = 0x00;							/* External oscillation control register, Normal performance */

	PLLCNT &= (UCHAR)(~(BP_BIT6 | BP_BIT7));	/* Clock Multuplication Circuit Control register */
	PLLCNT |= (UCHAR)(BP_BIT4 | BP_BIT5);		/* x4 multiplication setting PLL (16MHz)? */

	PLLCNT |= BP_BIT0;						    /* Start PLL operation, PLLSTART = 1 */

	for(j = 0; j <= 400;  j++){					/* Wait for 100us after setting PLLSTART, for 4MHz */
		asm("	nop		;\n");
	}

	PLLCNT |= BP_BIT1;					        /* PLL enable */
#else	/* else of USE_EXTERNAL_OSC	*/
	OSCCNT = 0x10;								/* Set HOSCDIV to STOP */
	CPUM   = 0x01;								/* NORMAL-IDLE */
	RCCNT  = 0x82;								/* Select internal High Speed 16MHz */

	asm("	NOP");

	CPUM = 0x00;								/* NORMAL MODE */

	CPUM = 0x10; 								/* OSCDBL Double speed (fpll) */

	OSCCNT = 0x00;								/* Set HOSCDIV to active, no divide fpll-div */
	PLLCNT = 0x00;								/* PLL stop, OSC mode */
#endif	/* end of USE_EXTERNAL_OSC */

	CPUM &= (UCHAR)(~(BP_BIT5 | BP_BIT6));		/* OSCSEL div ratio = 1 */
	CPUM |= BP_BIT4;							/* Internal system clock (fs) as (fpll) */

	asm("	or	0x70,psw	");					/* IM = "11", MIE = '1' */

	IRQ2ICR =  0x00;							/* Clear / Disable first */	
	NF2CTR  =  0xD0;							/* Noise filter fpll/2^10 */
	EDGDT  &= ~BP_BIT2;							/* Programmable edge based on IRQ2ICR */
	
	IRQCNT  =  0x04;							/* Enable P22 external interrupt */	
	IRQ2ICR =  0x42;							/* Enable IRQ2 interrupt, negedge detect */	
}	


/******************************************************************
#ifdef DOC
FUNCTION	: vMAIN_GetSWInputs
DESCRIPTION	: Capture system configuration switch inputs
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
******************************************************************/
void vMAIN_GetSWInputs( void )
{
/* Need only to set one ID/Msg Buffer as TX, since no automatic RX-Response-TX */
/* BRD_MODE |	Designation | Motor |  ID	| Sensor |  ID   */
/* 	  0		|	Master brd  |   Tx	| 0x100 |	RX	 | 0x300 */
/* 	  1		|	Slave brd   |   Rx	| 0x200 |	TX	 | 0x400 */

/* SW1_IN: BRD_MODE P62 input */
/* ucBRD_MODE = (UCHAR)((P6IN & 0x04) >> 2); */
  ucBRD_MODE = BRD_MODE;
/* SW2_IN: SYS_BUS P63 input */
/* ucSYS_BUS  = (UCHAR)((P6IN & 0x04) >> 3); */
  ucSYS_BUS  = SYS_BUS;
  
  if( ucBRD_MODE )
  {
	usCAN_TxID   = CAN_RCVID_ID4;
	usCAN_DestID = CAN_RCVID_ID3;
  }
  else
  {
  	usCAN_TxID   = CAN_RCVID_ID1;
	usCAN_DestID = CAN_RCVID_ID2;
  }
}

/******************************************************************
#ifdef DOC
FUNCTION	: vMAIN_StartMainTimer
DESCRIPTION	: Timer initialization for Timer 2 (for main polling timeout)
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
******************************************************************/
void vMAIN_StartMainTimer( void )
{
	TM2MD  &= (UCHAR)(~BP_BIT3);				/* Clock Stop */
	TM2ICR &= (UCHAR)(~BP_BIT1);				/* IRQ disable */
	CK2MD   =  0x08;							/* fpll-div/128 */
	TM2MD  |=  BP_BIT0;							/* NormalTimer,TM2PSC */
	TM2MD  &= (UCHAR)(~(BP_BIT5|BP_BIT4|BP_BIT1));
	TM2OC   =  0xFF;
	TM2MD  |=  BP_BIT3;							/* Clock Start */

	TMRICR  = 0x00;								/* Disable interrupt */

	TMRMD1  = 0x00;								/* Default settings */
	TMRMD2  = 0x0A;								/* fpll = 5MHz, n = 4 */
	TMRBCL  = 0x00;								/* Initialize binary counter */
	TMRBCM  = 0x00;
	TMRBCH  = 0x00;
	TMRMD1  = 0x10;								/* Generate interrupt every 1sec */
	TMRMD1  = 0x11;								/* Enable 24H timer operation */
	
	TMRICR  = 0x42;								/* Enable interrupt */
}	

/******************************************************************
#ifdef DOC
FUNCTION	: ucMAIN_GetMainTimer
DESCRIPTION	: Timer initialization for Timer 2 (for main polling timeout)
INPUT		: void
OUTPUT		: (UCHAR) TM2BC
RETURN		: 
#endif
******************************************************************/
UCHAR ucMAIN_GetMainTimer( void )
{
	return( TM2BC );
}

/******************************************************************
#ifdef DOC
FUNCTION	: vMAIN_StartMainCounter
DESCRIPTION	: Timer initialization for Timer 2 (for main polling timeout)
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
******************************************************************/
void vMAIN_StartMainCounter( void )
{
	ucMAIN_LoopStartCount = ucMAIN_GetMainTimer();
}

/******************************************************************
#ifdef DOC
FUNCTION	: vMAIN_WaitMainCounter
DESCRIPTION	: Loop for 1ms
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
******************************************************************/
void vMAIN_WaitMainCounter( void )
{
	UCHAR now_count;
	UCHAR elapsed_count;
	
	for(;;)
	{
	  now_count = ucMAIN_GetMainTimer();
	  elapsed_count = now_count - ucMAIN_LoopStartCount;
	  if (elapsed_count >= (UCHAR)MAIN_LOOP_COUNT)
	    {
	      ucMAIN_LoopStartCount += MAIN_LOOP_COUNT;
	      return;
	    }
	}
}

/******************************************************************
#ifdef DOC
FUNCTION	: vMAIN_init7seg
DESCRIPTION	: Set 7seg related ports
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
******************************************************************/
void vMAIN_Init7Seg( void )
{
	P8OUT = 0xFF;
	P8DIR = 0xFF;
	PAOUT = 0xFF;
	PADIR = 0xFF;
}

/******************************************************************
#ifdef DOC
FUNCTION	: vMAIN_Disp7Seg
DESCRIPTION	: Display to 7segment
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
******************************************************************/
void vMAIN_Disp7Seg( UCHAR ucDisp_val )
{
	P8OUT = ~LED_7SEG[ucDisp_val/10];
	PAOUT = ~LED_7SEG[ucDisp_val%10];
}


/******************************************************************
#ifdef DOC
FUNCTION	: vMAIN_CANTxFrame
DESCRIPTION	: Create CAN frame for transmission
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
******************************************************************/
void vMAIN_CANTxFrame( USHORT usMAIN_TxID, UCHAR ucMAIN_TxDLC, UCHAR ucMAIN_TxDat0, UCHAR ucMAIN_TxDat1 )
{
	ucMAIN_can_send_data[0] = (UCHAR)(usMAIN_TxID);			/* ID-L Lower  8bits */
	ucMAIN_can_send_data[1] = (UCHAR)(usMAIN_TxID >> 8);	/* ID-H Higher 3bits */
	ucMAIN_can_send_data[2] = ucMAIN_TxDLC;					/* For Motor == 1, else 2 */
	ucMAIN_can_send_data[3] = ucMAIN_TxDat0;
	ucMAIN_can_send_data[4] = ucMAIN_TxDat1;

	ucCAN_SendReq(&ucMAIN_can_send_data[0]);
}


/******************************************************************
#ifdef DOC
FUNCTION	: vMAIN_CANTxFrame
DESCRIPTION	: Create CAN frame for transmission
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
******************************************************************/
void vMAIN_LINTxFrame( UCHAR ucMAIN_TxID, UCHAR ucMAIN_TxHdrReq, UCHAR ucMAIN_TxDat0, UCHAR ucMAIN_TxDat1 )
{
	UCHAR	lp_cnt;

	for( lp_cnt = 0; lp_cnt < LIN_RES_VALUE; lp_cnt++ )
	{
		if( LIN_ResponsInfoTable[lp_cnt].info_flag.id == ucMAIN_TxID )
		{
			LIN_ComData[lp_cnt].byte[0] = ucMAIN_TxDat0;
			LIN_ComData[lp_cnt].byte[1] = ucMAIN_TxDat1;
			LIN_ComData[lp_cnt].data.tx_hdr_req = ucMAIN_TxHdrReq;
		}
	}
}
