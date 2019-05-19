/**********************************************************************************
#ifdef DOC
FILE NAME	: adc.h
DESCRIPTION : ADC header file
REMARK		: MCU, Date
#endif
**********************************************************************************/

#ifndef _ADC_H
#define _ADC_H

/***********************************
		Include File
***********************************/

/***********************************
		Macro Definition
***********************************/

/***********************************
		Variable Declaration
***********************************/

/******************************************
		Function Prototype Declaration
******************************************/
void  vADC_Init(void);
void  vADC_MainProcessing(void);
void  vADC_Start(void);
UCHAR ucADC_Convert(void);

#endif
