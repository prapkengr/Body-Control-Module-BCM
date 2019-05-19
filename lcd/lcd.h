/*********************************************************************/
/* CAN/LIN DEMO BOARD  for  MCU (StarterKit)                   */
/*********************************************************************/

#ifndef		_LCD_H_
#define		_LCD_H_

union un_pout {						/* PnOUT */
	unsigned char BYTE;				/* Byte Access */
	struct {						/* Bit  Access */
		unsigned char B0      :1;		/*  Bit 7 */
		unsigned char B1      :1;		/*  Bit 6 */
		unsigned char B2      :1;		/*  Bit 5 */
		unsigned char B3      :1;		/*  Bit 4 */
		unsigned char B4      :1;		/*  Bit 3 */
		unsigned char B5      :1;		/*  Bit 2 */
		unsigned char B6      :1;		/*  Bit 1 */
		unsigned char B7      :1;		/*  Bit 0 */
	} BIT;
	struct {						/* Bit  Access */
		unsigned char L       :4;		/*  Bit 7-4 */
		unsigned char H       :4;		/*  Bit 3-0 */
	} NIBBLE;
	struct {						/* Bit  Access */
		unsigned char LL      :2;		/*  Bit 7-6 */
		unsigned char LH      :2;		/*  Bit 5-4 */
		unsigned char HL      :2;		/*  Bit 3-2 */
		unsigned char HH      :2;		/*  Bit 1-0 */
	} BIT2;
};

#define P7		(*(volatile union un_pout *)0x03E77) /* P0 */
#define P5		(*(volatile union un_pout *)0x03E75) /* P5 */
#define P8		(*(volatile union un_pout *)0x03E78) /* P8 */

#define LCD_RS			P7.BIT.B7		/* LCD RS   OUTPUT BIT */
#define LCD_RW			P7.BIT.B4		/* LCD RW   OUTPUT BIT */
#define LCD_E			P5.BIT.B2		/* LCD E    OUTPUT BIT */
#define LCD_DATA_L		P7.NIBBLE.L		/* LCD DATA OUTPUT BIT */
#define LCD_DATA_H		P8.NIBBLE.H		/* LCD DATA OUTPUT BIT */
#define LCD_DATA_HL		P8.BIT2.HL		/* LCD DATA OUTPUT BIT */
#define LCD_DATA_HH		P5.BIT2.HH		/* LCD DATA OUTPUT BIT */

#define LCD_RS_CMD		0
#define LCD_RS_DATA		1
#define LCD_RW_WRITE	0
#define LCD_RW_READ		1

/* fpll-div/64 = 10.0MHz/64 = 6.4us */
#define LCD_WAIT_1MS	156				/* 1.0ms/6.4us = 156.25 = 157(1.0048ms) */
#define LCD_WAIT_37US	5				/* 37us/6.4us  =   5.78 =   6(38.4us) */


void init_lcd_io( void );
void lcd_init( void );
void lcd_puts(char *data, char line, char cursor);
void lcd_set_cursor(char line, char cursor);
void lcd_put_data(char data);
void lcd_put_cmd( char cmd );
void lcd_put_cmd0( char cmd );
void lcd_set_data( char data );
void lcd_clear(void);
void lcd_wait( void );
void lcd_wait_ms( unsigned int time );
void int_timer0( void );


#endif		/* _LCD_H_ */
