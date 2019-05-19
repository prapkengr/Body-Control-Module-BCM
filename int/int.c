/**********************************************************************************
#ifdef DOC
FILE NAME	: int.c
DESCRIPTION : Motor control source file
REMARK		: MCU
DATE		: 
#endif
**********************************************************************************/

/**********************************
		Include File
**********************************/
#include "int.h"

#include "../header/MCU.h"					/* Special register definition file */
#include "sys_type.h"

/***********************************
		Macro Definition
***********************************/
#define ANBUF01 __UMEM16(0x3FC8)

/***********************************
		External Declaration
***********************************/
extern UCHAR	ucADC_Buff_Ptr;
extern USHORT	ucADC_Buffer[3];
extern UCHAR 	ucBRD_MODE;			/* 0:master, 1:slave */

/***********************************
		Variable Declaration
***********************************/
UCHAR 			ucINT_enable_capture;
UCHAR			ucINT_toggle_motor;

/******************************************
		Function Prototype Declaration
******************************************/


/****************************************************************************
#ifdef DOC
FUNCTION	: int_24h(void)
DESCRIPTION	: 24h timer interrupt service routine
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
*****************************************************************************/
#pragma _interrupt int_24h
void int_24h( void )
{
	TMRICR = 0x00;				/* Disable / Clear interrupt */
	TMRICR = 0x42;				/* Enable interrupt */
}


/****************************************************************************
#ifdef DOC
FUNCTION	: int_irq2(void)
DESCRIPTION	: IRQ2 (P22) interrupt service routine
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
*****************************************************************************/
#pragma _interrupt int_irq2
void int_irq2( void )
{
  IRQ2ICR = 0x00;				/* Disable / Clear interrupt */

  if( ucBRD_MODE ) {
	if( ucINT_enable_capture ){
		ucINT_enable_capture = 0;
	}
	else {
		ucINT_enable_capture = 1;
	}
  }
  else {
	ucINT_toggle_motor = 1;
  }

  IRQ2ICR = 0x42;				/* Enable interrupt */
}


/****************************************************************************
#ifdef DOC
FUNCTION	: int_adc(void)
DESCRIPTION	: ADC interrupt service routine
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
*****************************************************************************/
#pragma _interrupt int_adc
void int_adc( void )
{
	USHORT	ANBUF_tmp;

	IRQEXPEN &= ~BP_BIT5;		/* Disable AD interrupt */
	IRQEXPDT |=  BP_BIT5;		/* Clear interrupt */

	ANBUF_tmp = ANBUF01;		/* Read ANBUF stored value */
	ucADC_Buffer[ucADC_Buff_Ptr] = (ANBUF_tmp >> 6);
	ucADC_Buff_Ptr++;

	IRQEXPEN |=  BP_BIT5;		/* Enable AD interrupt */
}
