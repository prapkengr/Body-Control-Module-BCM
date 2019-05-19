/**********************************************************************************
#ifdef DOC
FILE NAME	: sys_type.h
DESCRIPTION : System definition header file
REMARK		: MCU
DATE		: 
#endif
**********************************************************************************/

/* Declare variable type definition*/
typedef void			VOID;
typedef unsigned char	UCHAR;
typedef unsigned short	USHORT;
typedef unsigned long	ULONG;
typedef signed char		SCHAR;
typedef signed short	SSHORT;
typedef signed long		SLONG;
typedef UCHAR			BOOL;

/* Bit position definition */
#define	BP_BIT0			(0x01)
#define	BP_BIT1			(0x02)
#define	BP_BIT2			(0x04)
#define	BP_BIT3			(0x08)
#define	BP_BIT4			(0x10)
#define	BP_BIT5			(0x20)
#define	BP_BIT6			(0x40)
#define	BP_BIT7			(0x80)

#define Hall_DevID		(0x00)
#define ADC_DevID		(0x01)
#define PWM_DevID		(0x02)
#define MOTOR_DevID		(0x03)

#define LED_DevID		(0x00)
#define Buzz_DevID		(0x01)
#define LCD_DevID		(0x02)

/* Characteristic/function of each node (default master)	*/
/* BRD_MODE |	Designation | Motor |  ID  |  Sensor |  ID  */
/* 	  0		|	Master brd  |   Tx	| 0x10 |	RX	 | 0x20 */
/* 	  1		|	Slave brd   |   Rx	| 0x10 |	TX	 | 0x20 */
#define	LIN_MOTOR_ID	(0x10)
#define	LIN_SENSOR_ID	(0x20)

typedef union {					
	unsigned char BYTE;				/* Byte Access */
	struct {						/* Bit  Access */
		unsigned char MotID   :4;		/*  Bit 3-0 */
		unsigned char MDir    :2;		/*  Bit 5-4 */
		unsigned char Spd     :1;		/*  Bit 6 */
		unsigned char Rst     :1;		/*  Bit 7 */
	} MOTOR;
	struct {						/* Bit  Access */
		unsigned char SnsID   :4;		/*  Bit 3-0 */
		unsigned char Rsv4    :1;		/*  Bit 4 */
		unsigned char Rsv5    :1;		/*  Bit 5 */
		unsigned char State   :1;		/*  Bit 6 */
		unsigned char Err     :1;		/*  Bit 7 */
	} SENSOR;
}FRAME0;

/******************************************
		Function Prototype Declaration
******************************************/
void	main( void );
void	vMAIN_Initialize( void );
void	vMAIN_InitializeIO( void );
void 	vMAIN_GetSWInputs( void );
void	vMAIN_SystemClockSetting( void );
void	vMAIN_StartMainTimer( void );
UCHAR	ucMAIN_GetMainTimer( void );
void	vMAIN_StartMainCounter( void );
void	vMAIN_WaitMainCounter( void );
void 	vMAIN_Init7Seg( void );
void 	vMAIN_Disp7Seg( UCHAR ucDisp_val );
void 	vMAIN_CANTxFrame( USHORT usMAIN_TxID, 
						UCHAR ucMAIN_TxDLC, 
						UCHAR ucMAIN_TxDat0, 
						UCHAR ucMAIN_TxDat1 );
void 	vMAIN_LINTxFrame( UCHAR ucMAIN_TxID, 
						UCHAR ucMAIN_TxHdrReq, 
						UCHAR ucMAIN_TxDat0, 
						UCHAR ucMAIN_TxDat1 );
