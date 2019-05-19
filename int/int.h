/**********************************************************************************
#ifdef DOC
FILE NAME	: int.h
DESCRIPTION : Interrupt header file
REMARK		: MCU
DATE		: 
#endif
**********************************************************************************/

#ifndef		_INT_H_
#define		_INT_H_

/***********************************
		Include File
***********************************/

/***********************************
		Macro Definition
***********************************/

/******************************************
		Function Prototype Declaration
******************************************/
void int_24h( void );
void int_irq2( void );
void int_adc( void );

#endif
