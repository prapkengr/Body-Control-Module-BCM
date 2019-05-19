/**********************************************************************************
#ifdef DOC
FILE NAME	: motor.c
DESCRIPTION : Motor control source file
REMARK		:
DATE		: 
#endif
**********************************************************************************/

/***********************************
		Include File
***********************************/
#include "sys_type.h"
#include "MCU.h"
#include "motor.h"


/***********************************
		External Declaration
***********************************/

/***********************************
		Macro Definition
***********************************/
#define DEBUG_MODE 		1

/***********************************
		Variable Declaration
***********************************/
UCHAR   		ucMOTOR_state;
UCHAR			ucMOTOR_DIR_state;
extern 	UCHAR	ucINT_toggle_motor;

/******************************************
		Function Prototype Declaration
******************************************/
void vMOTOR_Init(void);

/****************************************************************************
#ifdef DOC
FUNCTION	: void vMOTOR_Init(void)
DESCRIPTION	: Initialization for Motor control related registers
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
*****************************************************************************/
void vMOTOR_Init(void)
{
#if !DEBUG_MODE
  /* Initialize IO ports for motor control */
  P8OUT   =  0x00;

/* Set P80-P83, P86-P87 direction to output */
  P8DIR   =  0xCF;
    /* P87 : DECAY[1] = 0 */
    /* P86 : DECAY[0] = 0 */
    /* P85 : N.C.         */
    /* P84 : N.C          */
    /* P83 : DIR = 0      */
    /* P82 : ST3 = 0      */
    /* P81 : ST2 = 0      */
    /* P80 : ST1 = 0      */
#endif
  
  P5OUT  &=  0xFC;
  P5DIR  |= (UCHAR)(BP_BIT1 | BP_BIT0);
    /* P51 : ENABLE = 0   */
    /* P50 : RESET  = 0   */

  /* Setup Timer 0 to generate PWM output to TM0IOB port used as CLK for motor driver */
  TM0MD  &= ~BP_BIT3;      /* Stop Timer counting */

  P0OMD1 |= BP_BIT4;       /* Set timer output function for P04 */
  P0OMD2  = 0x00;          /* Select TM0IOB function for P04 */
  P0DIR  |= (UCHAR)(BP_BIT5 | BP_BIT4);      /* Set P04 direction to output */

  TM0MD  |=  BP_BIT0;      /* Set TM0PSC as clock source */
  TM0MD  &= ~BP_BIT1;
  
  /* Set TM0BAS = 1 */
/*  CK0MD  |= BP_BIT0; */
  CK0MD  &= ~BP_BIT0;

  /* Select fs/2 */
/*  CK0MD  &= (UCHAR)(0xFF^(BP_BIT3 | BP_BIT2 | BP_BIT1)); */
  CK0MD  |=  (UCHAR)(BP_BIT3 | BP_BIT2 | BP_BIT1);
  
  TM0OC   =  COMPARE_COUNT;

  TM0MD  |=  BP_BIT3;      /* Start Timer counting operation */
  
  P5OUT  |=  BP_BIT0;
    /* P51 : ENABLE = 0   */
    /* P50 : RESET  = 1   */
 
  ucMOTOR_state     = MOTOR_IDLE;
  ucMOTOR_DIR_state = MOTOR_RVS;
}

/****************************************************************************
#ifdef DOC
FUNCTION	: void vMOTOR_MasterMain(void)
DESCRIPTION	: Main function for master baord
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
*****************************************************************************/
void vMOTOR_MasterMain(void)
{
  if( ucINT_toggle_motor ){
    switch( ucMOTOR_state )
    {
      default:
      { 
		ucMOTOR_state = MOTOR_FWD;
		break;
      }
      case MOTOR_IDLE:
      {
/*		vMOTOR_Idle(); */
		if(ucMOTOR_DIR_state == MOTOR_RVS){
	 		ucMOTOR_state = MOTOR_FWD;
		}
	 	else{
	 		ucMOTOR_state = MOTOR_RVS;
	 	}
        break;
      } 
      case MOTOR_FWD:
      {
/*		vMOTOR_Start(MOTOR_FWD); */
		ucMOTOR_state = MOTOR_IDLE;
		ucMOTOR_DIR_state = MOTOR_FWD;
        break;
      } 
      case MOTOR_RVS:
      {
/*		vMOTOR_Start(MOTOR_RVS); */
	 	ucMOTOR_state = MOTOR_IDLE;
		ucMOTOR_DIR_state = MOTOR_RVS;
        break;
      } 
    }
	ucINT_toggle_motor = 0;
  }

}

/****************************************************************************
#ifdef DOC
FUNCTION	: void vMOTOR_Start(void)
DESCRIPTION	: Operate the motor 
INPUT		: UCHAR ucMDir
OUTPUT		: void
RETURN		: void
#endif
*****************************************************************************/
void vMOTOR_Start(UCHAR ucMDir)
{
  USHORT i,j;

  P0OMD1 |= BP_BIT4;       /* Set timer output function for P04 */

#if DEBUG_MODE
  if(ucMDir == MOTOR_FWD){
	P5OUT  &= ~BP_BIT1;
  }
  else{
	P5OUT  |=  BP_BIT1;
  }
#else
  if(ucMDir == MOTOR_FWD){
    P8OUT  |=  MOTOR_FWD;    /* P83 : DIR = 1      */
  }
  else{
    P8OUT  &=  MOTOR_RVS;    /* P83 : DIR = 0      */    
  }
#endif

  for(i = 0; i <= 10;  i++){	/* Wait for 2us after setting DIR before CLK, for 5MHz */
    asm("	nop		;\n");
  }

 TM0MD  |=  BP_BIT3;      /* Start Timer counting */

  for(j = 0; j <= 10;  j++){
    asm("	nop		;\n");
  }

 P5OUT  |=  BP_BIT0;  /* Enable motor */
 
}

/****************************************************************************
#ifdef DOC
FUNCTION	: void vMOTOR_Idle(void)
DESCRIPTION	: Put the motor on standby
INPUT		: void
OUTPUT		: void
RETURN		: void
#endif
*****************************************************************************/
void vMOTOR_Idle(void)
{
  P5OUT  &= ~BP_BIT0;      /* Disable motor */

  TM0MD  &= ~BP_BIT3;      /* Stop Timer counting */

  P0OMD1 &= ~BP_BIT4;      /* Set output of P04 to 0 */
}
