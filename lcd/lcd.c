/*********************************************************************/
/* CAN/LIN DEMO BOARD  for  MCU (StarterKit)                   */
/*                                */
/*********************************************************************/

#include "MCU.h"			/* Special register definition file */
#include "lcd.h"

#define STK_LCD4BIT		1			/* LCD I / F mode : 0=8bit, 1=4bit */
#define STK_LCDDPORT	1			/* LCD DATA7-4   : 0=P87-P84, 1=P57-P56,P85-P84 */

unsigned int lcd_time;				/* LCD : Timer count */


/*+Func Public============================================
|
Function name      : init_lcd_io
+---------------------------------------------------------
|LCD system I/O initialization
+---------------------------------------------------------
|Argument: None
+---------------------------------------------------------
|Return value: None
+---------------------------------------------------------
|Functional Description: Initializes the LCD control I / O.
+=======================================================*/
/*  Initialization of LCD system I/O*/
void init_lcd_io( void )
{
	/* Timer for LCD Wait */
	TM4ICR = 0x00; / * Disable interrupts * /
	TM4MD = 0x00; / * Count stop * /
	TM4MD = 0x01; / * Clock source (prescaler output) * /
	CK4MD = 0x06; / * Prescaler (fpll-div / 64) * /

	/* I/O Port */
	P5DIR | = 0x04; / * LCD control (E) * /
	P7DIR | = 0x9F; / * LCD control (RS / RW / DATA3-0) * /
#if STK_LCD4BIT
	P7DIR  |= 0x90;
#else
	P7DIR  |= 0x9F;
#endif
#if STK_LCDDPORT
	P5DIR | = 0xC0; / * LCD control (DATA 7-6) * /
	P8DIR | = 0x30; / * LCD control (DATA 5-4) * /
#else
	P8DIR | = 0xF0; / * LCD control (DATA 7-4) * /
#endif

	/* Initialize for LCD control port */
	LCD_E  = 0;
	LCD_RS = LCD_RS_CMD;
	LCD_RW = LCD_RW_WRITE;
}


/*+Func Public============================================
|Name of function: lcd_init
+---------------------------------------------------------
|LCD Initialization and Startup Message Display
+---------------------------------------------------------
|Argument: None
+---------------------------------------------------------
|Return value: None
+---------------------------------------------------------
|Functional Description: Initialize LCD and display startup message
+=======================================================*/
/* LCD initialization & start message display */
void lcd_init( void )
{
	lcd_wait_ms( 30 );
	lcd_put_cmd0( 0x30 );			/* Function set: 8bit */
	lcd_wait_ms( 4 );
	lcd_put_cmd0( 0x30 );			/* Function set: 8bit */
	lcd_wait();
	lcd_wait();
	lcd_wait();
	lcd_put_cmd0( 0x30 );			/* Function set: 8bit */
#if STK_LCD4BIT
	lcd_put_cmd0( 0x20 );			/* Function set: 4bit */
	lcd_put_cmd( 0x2C );			/* Function set: 4bit, 2lines, big char */
#else
	lcd_put_cmd0( 0x3C );			/* Function set: 8bit, 2lines, big char */
#endif
	lcd_put_cmd( 0x08 );			/* Display Off: D=C=B=0 */
	lcd_put_cmd( 0x0C );			/* Display On: D=1, C=B=0 */
	lcd_put_cmd( 0x06 );			/* Entry Mode Set: incremental, without display shift */

	lcd_clear();
	lcd_clear();
	lcd_wait();
	lcd_wait();
	lcd_puts( " LIN/CAN Demo Board ", 0, 0 );
	lcd_puts( "   for MCU 1   ", 1, 0 );
	lcd_puts( "--------------------", 2, 0 );
	lcd_puts( " SW 1:  2:  3:  4:  ", 3, 0 );
}


/*+Func Public============================================
|Function Name: lcd_puts
+---------------------------------------------------------
|String Display
+---------------------------------------------------------
| Arguments: data: Display string pointer. Exit is NULL
|: pos_y: Vertical display position (0 at the top, 3 at the bottom)
|: pos_x: Horizontal display position (0 at the left end, 19 at the right end)
+---------------------------------------------------------
|Return value: None
+---------------------------------------------------------
|Function description: Display the character string from the specified display position
+=======================================================*/
void lcd_puts( char *data, char pos_y, char pos_x )
{
	lcd_set_cursor( pos_y, pos_x );
	while( *data ){
		lcd_put_data( *data++ );
	}
}


/*+Func Public============================================
|Function name: lcd_set_cursor
+---------------------------------------------------------
|Display start position setting
+---------------------------------------------------------
| Argument: pos_y: Vertical position (0 at the top, 3 at the bottom)
|: pos_x: Horizontal position (0 at the left end, 19 at the right end)
+---------------------------------------------------------
|Return value: None
+---------------------------------------------------------
|Function Description: Set cursor at specified position
+=======================================================*/
void lcd_set_cursor( char pos_y, char pos_x )
{
	switch( pos_y ){
		case 0: pos_x += 0x00; break;
		case 1: pos_x += 0x40; break;
		case 2: pos_x += 0x14; break;
		case 3: pos_x += 0x54; break;
	}
	lcd_put_cmd( 0x80|pos_x );
}


/*+Func Public============================================
|Function name: lcd_put_data
Character Data Setting
+ --------------------------------------------------------- --------
| Argument: data: Character (LCD ASCII code)
+ --------------------------------------------------------- --------
| Return value: None
+ --------------------------------------------------------- --------
| Function Description: Set the character
+=======================================================*/
void lcd_put_data( char data )
{
	LCD_RS = LCD_RS_DATA;
#if STK_LCD4BIT
	lcd_set_data( data );			/* Data (upper 4 bits) */
	LCD_E  = 1;
	LCD_E  = 0;
	lcd_wait();
	lcd_set_data( data<<4 );		/* Data (lower 4 bits) */
	LCD_E  = 1;
	LCD_E  = 0;
	lcd_wait();
#else
	lcd_set_data( data );			/*  data (8 bits) */
	LCD_E  = 1;
	LCD_E  = 0;
	lcd_wait();
#endif
}


/* + Func Public ======================================================
| Function Name: lcd_put_cmd
+ --------------------------------------------------------- --------
| 	Control Command Settings
+ --------------------------------------------------------- --------
| Argument: cmd: Control command
+ --------------------------------------------------------- --------
| Return value: None
+ --------------------------------------------------------- --------
Description of function: Set control command (2 times of upper and lower for 4 bit bus)
+ ============================================================ ====== **/
void lcd_put_cmd( char cmd )
{
	LCD_RS = LCD_RS_CMD;
#if STK_LCD4BIT
	lcd_set_data( cmd );			/* Command (upper 4 bits) */
	LCD_E  = 1;
	LCD_E  = 0;
	lcd_wait();
	lcd_set_data( cmd<<4 );			/*  Command (lower 4 bits)  */
	LCD_E  = 1;
	LCD_E  = 0;
	lcd_wait();
#else
	lcd_set_data( cmd );			/* Command (8bit)  */
	LCD_E  = 1;
	LCD_E  = 0;
	lcd_wait();
#endif
}


/* + Func Public ======================================================
| Function name: lcd_put_cmd0
+ --------------------------------------------------------- --------
| Control Command Setting 0
+ --------------------------------------------------------- --------
| Argument: cmd: Control command
+ --------------------------------------------------------- --------
| Return value: None
+ --------------------------------------------------------- --------
Function description: Set control command (only during initialization operation)
+ ============================================================ ====== **/
void lcd_put_cmd0( char cmd )
{
#if STK_LCD4BIT
	LCD_RS = LCD_RS_CMD;
	lcd_set_data( cmd );			/* Command (upper 4 bits) */
	LCD_E  = 1;
	LCD_E  = 0;
	lcd_wait();
#else
	lcd_put_cmd( cmd );
#endif
}


/* + Func Public ======================================================
| Function name: lcd_set_data
+ --------------------------------------------------------- --------
|  Data Settings
+ --------------------------------------------------------- --------
| Argument: data: character / command
+ --------------------------------------------------------- --------
| Return value: None
+ --------------------------------------------------------- --------
Functional Description: Set data to data port
+
+=======================================================*/
void lcd_set_data( char data )
{
#if STK_LCDDPORT
	LCD_DATA_HH = (data&0xC0)>>6;
	LCD_DATA_HL = (data&0x30)>>4;
#else
	LCD_DATA_H  = data>>4;
#endif
#if !STK_LCD4BIT
	LCD_DATA_L  = data;
#endif
}


/* + Func Public ======================================================
| Function name: lcd_clear
+ --------------------------------------------------------- --------
| Japanese Name: Display Clear
+ --------------------------------------------------------- --------
| Argument: None
+ --------------------------------------------------------- --------
| Return value: None
+ --------------------------------------------------------- --------
Functional Description: Clear the display
+=======================================================*/
void lcd_clear( void )
{
	lcd_put_cmd( 0x01 );			/* Clear display */
	lcd_wait_ms( 2 );
}


/* + Func Public ======================================================
| Function name: lcd_wait
+ --------------------------------------------------------- --------
| Japanese Name: Weight
+ --------------------------------------------------------- --------
| Argument: None
+ --------------------------------------------------------- --------
| Return value: None
+ --------------------------------------------------------- --------
Functional Description: Weight 37us (38.4us)
+=======================================================*/
void lcd_wait( void )
{
	lcd_time = 0;
	TM4OC = LCD_WAIT_37US; /* Compare register setting: 37us */
	TM4MD = 0x09; /* Timer start */
	TM4ICR = 0x02; /* Interrupt enabled */
	while( 1 ){
		if( lcd_time ) break;
	}
	TM4MD = 0x00; /* Timer stop */
}


/* + Func Public ======================================================
| Function name: lcd_wait_ms
+ --------------------------------------------------------- --------
| Japanese Name: Weight ms
+ --------------------------------------------------------- --------
| Argument: time: Wait time (ms)
+ --------------------------------------------------------- --------
| Return value: None
+ --------------------------------------------------------- --------
Functional Description: Wait in units of 1.0 ms (1.0048 ms)
+=======================================================*/
void lcd_wait_ms( unsigned int time )
{
	lcd_time = 0;
	TM4OC = LCD_WAIT_1MS; /* Compare register setting: 1 ms */
	TM4MD = 0x09; /* Timer start */
	TM4ICR = 0x02; /* Interrupt enabled */
	while( 1 ){
		if( lcd_time >= time ) break;
	}
	TM4MD = 0x00;					/* Timer stop */
}


