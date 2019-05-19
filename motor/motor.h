/**********************************************************************************
#ifdef DOC
FILE NAME	: motor.h
DESCRIPTION : Motor control header file
REMARK		:
DATE		: 
#endif
**********************************************************************************/

#ifndef _MOTOR_H
#define _MOTOR_H

/***********************************
		Include File
***********************************/

/***********************************
		Macro Definition
***********************************/
#define COMPARE_COUNT	(0x31)        /* d'49 for 100kHZ to be divided by 5MHz */
#define MOTOR_FWD 	(0x01)
#define MOTOR_RVS 	(0x02)
#define MOTOR_IDLE 	(0x03)

/***********************************
		Variable Declaration
***********************************/

/******************************************
		Function Prototype Declaration
******************************************/
void vMOTOR_Init(void);
void vMOTOR_MasterMain(void);
void vMOTOR_Start(UCHAR ucMDir);
void vMOTOR_Idle(void);

#endif
