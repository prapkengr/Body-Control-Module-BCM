/**********************************************************************************
#ifdef DOC
FILE NAME	: adc.c
DESCRIPTION : ADC source file
REMARK		: MCU
DATE		: 
#endif
**********************************************************************************/

/***********************************
		Include File
***********************************/
#include "sys_type.h"
#include "adc.h"


/***********************************
		External Declaration
***********************************/
extern UCHAR	ucSYS_BUS;
extern UCHAR 	ucINT_enable_capture;
extern USHORT	usCAN_DestID;

/***********************************
		Macro Definition
***********************************/
#define	ADC_LOOP_MS		(24)

/***********************************
		Variable Declaration
***********************************/
UCHAR			ucADC_Buff_Ptr;
USHORT			ucADC_Buffer[3];
static UCHAR	ucMAIN_ADC_loop;
FRAME0			ADC_FRAME0;

/******************************************
		Function Prototype Declaration
******************************************/

/****************************************************************************
#ifdef DOC
FUNCTION	: void vADC_Init(void)
DESCRIPTION	: Initialization for ADC control related registers
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
*****************************************************************************/
void vADC_Init(void)
{
	P5IMD    |= BP_BIT7;		/* Set P57 port as AN9 analog input */

	ANCTR1    = 0x09;			/* Select AN9 input channel  */
	ANCTR0    = 0x00;
	    /* ANSH   :  00, Tad x 2 */
		/* ANCK   : 000, fs/2    */
	    /* ANLADE :   0, OFF      */

	IRQEXPDT  = 0x00;			/* Clear interrupt */
	PERIICR   = 0x80;			/* PERIIRQ interrupt level '10' */
	IRQEXPEN |= BP_BIT5;		/* Enable AD interrupt */
	
	ANCTR0   |= BP_BIT3;	    /* ANLADE : 1, ON  */
	ANCTR2    = 0x00;
	    /* ANST   :  0, Clear */
		/* ANSTSEL: 00, direct setting of ANST */

	ucADC_Buff_Ptr  		= 0;
	ucMAIN_ADC_loop 		= 0;
	ucINT_enable_capture	= 0;

	ADC_FRAME0.SENSOR.SnsID = ADC_DevID;
	ADC_FRAME0.SENSOR.Rsv4  = 0;
	ADC_FRAME0.SENSOR.Rsv5  = 0;
	ADC_FRAME0.SENSOR.State = 0;
	ADC_FRAME0.SENSOR.Err   = 0;
}

/****************************************************************************
#ifdef DOC
FUNCTION	: vADC_MainProcessing
DESCRIPTION	: Main module for ADC function
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
*****************************************************************************/
void vADC_MainProcessing(void)
{
	UCHAR	ucMAIN_ADC_val  =  0;

	if( ucINT_enable_capture )
	{
	    if( ucADC_Buff_Ptr >= 3 ){
    		ucMAIN_ADC_val = ucADC_Convert();
			
			if( ucSYS_BUS ){
				vMAIN_CANTxFrame( usCAN_DestID, 0x02, ADC_FRAME0.BYTE, ucMAIN_ADC_val );
			}
			else{
				vMAIN_LINTxFrame( LIN_SENSOR_ID, 0x00, ADC_FRAME0.BYTE, ucMAIN_ADC_val );
			}

    		ucADC_Buff_Ptr = 0;
	    }

		if( ucMAIN_ADC_loop >= ADC_LOOP_MS ){
	    	vADC_Start();
			ucMAIN_ADC_loop = 0;
		}
		else{
			ucMAIN_ADC_loop++;
		}
  	}
	else
	{
   		ucADC_Buff_Ptr  = 0;
		ucMAIN_ADC_loop = 0;
	}
}

/****************************************************************************
#ifdef DOC
FUNCTION	: void vADC_Start(void)
DESCRIPTION	: Start analog capture
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
*****************************************************************************/
void vADC_Start(void)
{
	ANCTR2   |= BP_BIT7;		/* ANST : 1, Start */
}

/****************************************************************************
#ifdef DOC
FUNCTION	: void vADC_Convert(void)
DESCRIPTION	: Convert 10bit measured value to [00:99]
INPUT		: void
OUTPUT		: void
RETURN		: UCHAR
#endif
*****************************************************************************/
UCHAR ucADC_Convert(void)
{
	USHORT	usADC_average;
	UCHAR	ucADC_result;

	usADC_average = (ucADC_Buffer[0] + ucADC_Buffer[1] + ucADC_Buffer[2])/3;
	
	if( usADC_average >= 990 ){
		ucADC_result  = 99;
	}
	else{
		ucADC_result  = (UCHAR)(usADC_average/10);
	}
	
	return( ucADC_result );
}

