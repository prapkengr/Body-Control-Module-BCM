;;/**********************************************************************************/
;;/* File Name		: Startup.asm													*/
;;/* Description	: 																*/
;;/* AUTHORS		: 																*/
;;/* DATE			: 																*/
;;/**********************************************************************************/

;-- Define Control Registers --

PRTKEY		equ	0x3E50
WDCTR		equ	0x3F02
WD2MDSEL	equ	0x3F4C


_STEXT		SECTION	CODE,PUBLIC,0
_CONST		SECTION	CODE,PUBLIC,1
_GCONST		SECTION	CODE,PUBLIC,1
_ROMDATA	SECTION	CODE,PUBLIC,1
_GROMDATA	SECTION	CODE,PUBLIC,1
_TEXT		SECTION	CODE,PUBLIC,0
_CODEEND	SECTION	CODE,PUBLIC,1
_DATA		SECTION	DATA,PUBLIC,1
_GDATA		SECTION	DATA,PUBLIC,1
_BSS		SECTION	DATA,PUBLIC,1
_GBSS		SECTION	DATA,PUBLIC,1
_BSSEND		SECTION	DATA,PUBLIC,1

	GLOBAL	_main, _int_24h, _int_irq2, _int_adc, _vCAN_Interrupt

_STEXT		SECTION
	da	A(Reset)		;  0:0x4000: reset vector address
	da	A(NoIRQ)		;  1:0x4004: Non-maskable interrupt	Watch Dog (Interrupt)
	da	A(NoIRQ)		;  2:0x4008: External interrupt 0
	da	A(NoIRQ)		;  3:0x400C: External interrupt 1
	da	A(_int_irq2)		;  4:0x4010: External interrupt 2
	da	A(NoIRQ)		;  5:0x4014: External interrupt 3
	da	A(NoIRQ)		;  6:0x4018: External interrupt 4
	da	A(NoIRQ)		;  7:0x401C: LIN interrupt
	da	A(NoIRQ)		;  8:0x4020: Low power voltage interrupt
	da	A(NoIRQ)		;  9:0x4024: Timer 0 interrupt
	da	A(NoIRQ)		; 10:0x4028: Timer 1 interrupt
	da	A(NoIRQ)		; 11:0x402C: Timer 2 interrupt
	da	A(NoIRQ)		; 12:0x4030: Timer 3 interrupt
	da	A(NoIRQ)		; 13:0x4034: Timer 4 interrupt
	da	A(_int_24h)		; 14:0x4038: 24H interrupt
	da	A(NoIRQ)		; 15:0x403C: Timer 6 interrupt
	da	A(NoIRQ)		; 16:0x4040: Time base interrupt
	da	A(NoIRQ)		; 17:0x4044: Timer 7 interrupt
	da	A(NoIRQ)		; 18:0x4048: Timer 8 interrupt
	da	A(NoIRQ)		; 19:0x404C: Timer 9 compare register 2 match interrupt
	da	A(NoIRQ)		; 20:0x4050: Serial 0 interrupt
	da	A(NoIRQ)		; 21:0x4054: Serial 0 UART reception interrupt
	da	A(NoIRQ)		; 22:0x4058: Serial 1 interrupt
	da	A(NoIRQ)		; 23:0x405C: Serial 1 UART reception interrupt
	da	A(NoIRQ)		; 24:0x4060: Serial 2 interrupt
	da	A(NoIRQ)		; 25:0x4064: Serial 2 UART reception interrupt
	da	A(_vCAN_Interrupt)	; 26:0x4068: CAN interrupt
	da	A(NoIRQ)		; 27:0x406C: Serial 4 interrupt
	da	A(NoIRQ)		; 28:0x4070: Serial 4 stop condition interrupt
	da	A(NoIRQ)		; 29:0x4074: Peripheral function group
	da	A(_int_adc)		; 30:0x4078: interrupt vector ( Actual interrupt function name must be set. )
	da	A(MonInterrupt) 	; 31:0x407C: Moninterrupt (Don't Change! )

	org	0xC0			;  _STEXT + 0xC0 (0x40C0)

	DC	0xFD, 0x07		; Flash option setting (WD2MDSEL enabled)

	org	0xD0			; _STEXT + 0xd0 (0x40d0) address

_TEXT		SECTION
NoIRQ:					; Interruption which is not used
	rti

Reset:

;-- Initialize Stack Pointer --

	movw	0x2800,A0
	movw	A0,SP

;-- Stop Watch Dog Timer --

	mov	0x44,d0
	mov	d0,(PRTKEY)
	mov	0x00,d0
	mov	d0,(WDCTR)

	mov	0x88,d0
	mov	d0,(PRTKEY)
	mov	0x00,d0
	mov	d0,(WD2MDSEL)

	mov	0x00,d0
	mov	d0,(PRTKEY)

;-- Initialize RAM --
					; initialization of static variables
	movw	_BSS,A0			; set start address of _BSS domain to A0 register
	sub	D0,D0			; D0 = 0
clear1:
	mov	D0,(A0)
	addw	0x1,A0
	cmpw	_BSSEND,A0		; Is it the last of a RAM domain?
	blt	clear1			; if _BSSEND > A0, jump to clear1

					; Initialization of static variable with default value
raminit:
	movw	_ROMDATA,A0
	movw	_DATA,A1
	cmpw	_GROMDATA,A0
	beq	next1
init1:					; ROMDATA(A0) -> DATA(A1)
	mov	(A0),D0
	mov	D0,(A1)
	addw	0x1,A0
	addw	0x1,A1
	cmpw	_GROMDATA,A0
	bne	init1

next1:
	movw	_GROMDATA,A0
	movw	_GDATA,A1
	cmpw	_TEXT,A0
	beq	next2
init2:					; GROMDATA(A0) -> GDATA(A1)
	mov	(A0),D0
	mov	D0,(A1)
	addw	0x1,A0
	addw	0x1,A1
	cmpw	_TEXT,A0
	bne	init2

;-- Initialize Register --

next2:			
	subw	DW0,DW0
	movw	DW0,DW1
	movw	DW0,A0
	movw	DW0,A1
				

;-- Branch Main loop --

_loop1:					
	jsr	_main

					; end of procedure
	pi				; A break will occur.
	rts


; Monitor

#if 1					; "0" when not using a monitor program.

;;;;; Monitor program from here. Don't change!
;;;;; Please don't set up the break point.

#define DEBUG_CMDFLG	0		; Fault correction of CMDFLG clear
#define OCDMONVER       0x0100
#define OCDRESERVE1     0x55AA
#define OCDRESERVE2     0x33CC
#define MONCT           0x3F0C
#define MONDT           0x3F0D
#define MONCTxx         0x3FFE

ChangeMonitorModeFlag   equ  0x20        ;Monitor Mode ON Flag
ChangeDataFlag          equ  0x40        ;
ChangeCommandFlag       equ  0x80        ;
ClearFlag               equ  0x02        ;CommandFlag and DataFlag  ClearFlag
CheckMonitorModeFlag    equ  0x40        ;Monitor Mode Check Flag
CheckCommandFlag        equ  0x20        ;Monitor and Checkcommand Flag
CheckDataFlag           equ  0x10        ;Monitor and Cheack Data Flag
CheckESCBreak           equ  0x02        ;ESC Break Flag  
CheckRomEvent           equ  0x04        ;ROM Event Flag
CheckRamEvent           equ  0x08        ;RAM Event Flag

	GLOBAL   MonInterrupt

_STEXT2      SECTION CODE,PUBLIC,0
_STEXT2      SECTION

OCDMonitorReserve:                       ; Monitor-code
    DW  OCDRESERVE1                      ; Monitor-code
    DW  OCDRESERVE2                      ; Monitor-code
    DW  OCDMONVER                        ; Monitor-version
    DW  OCDMonitorEnd - MonInterrupt     ; Monitor-code size
    
MonInterrupt:                            ; DW0,DW1,A0,A1 to SP
    addw    -8,SP                        ; Monitor-code
    movw    dw0,(6,SP)                   ; Monitor-code
    movw    dw1,(4,SP)                   ; Monitor-code
    movw    a0,(2,SP)                    ; Monitor-code
    movw    a1,(0,SP)                    ; Monitor-code

OCDSetMonitorModeFlag:                   ; SET MONITOR
    mov     (MONCT),d0                   ; Monitor-code
    btst    0x40,d0                      ; Monitor-code
    bne     OCDWaitDataFlag              ; Monitor-code
    mov     ChangeMonitorModeFlag,d0     ; Monitor-code
    mov     d0,(MONCT)                   ; Monitor-code
    
OCDWaitDataFlag:                         ; CHECK DataFlag
#if DEBUG_CMDFLG
    mov     0,(MONCTxx)
#endif
    mov     (MONCT),d0                   ; Monitor-code
    btst    CheckDataFlag,d0             ; Monitor-code
    beq     OCDWaitDataFlag              ; Monitor-code

OCDSetSPLow:                             ; SET SP_L
    movw    SP,a0                        ; Monitor-code
    movw    a0,dw0                       ; Monitor-code
    mov     d0,(MONDT)                   ; Monitor-code

OCDClearDataFlag:                        ; CLR DATA and COMMAND
    mov     ClearFlag,d0                 ; Monitor-code
    mov     d0,(MONCT)                   ; Monitor-code

OCDWaitDataFlag2:                        ; CHECK DataFlag
#if DEBUG_CMDFLG
    mov     0,(MONCTxx)
#endif
    mov     (MONCT),d0                   ; Monitor-code
    btst    CheckDataFlag,d0             ; Monitor-code
    beq     OCDWaitDataFlag2             ; Monitor-code

OCDSetSPHi:                              ; SET SP_H
    mov     d1,(MONDT)                   ; Monitor-code

OCDClearDataAndCommandFlag:              ; CLR DATA and COMMAND
    mov ClearFlag,d0                     ; Monitor-code
    mov d0,(MONCT)                       ; Monitor-code

OCDCommandLoop:                          ; LOOP MON
#if DEBUG_CMDFLG
    mov     0,(MONCTxx)
#endif
    mov     (MONCT),d0                   ; Monitor-code
    btst    CheckMonitorModeFlag,d0      ; Monitor-code
    beq     OCDGoProgram                 ; Monitor-code
    btst    CheckCommandFlag,d0          ; Monitor-code
    beq     OCDCommandLoop               ; Monitor-code

    nop                                  ; for debug

OCDChangeSP:                             ; CHANGE SP
    movw    (6,SP),a0                    ; Monitor-code
    movw    a0,sp                        ; Monitor-code
    bra     OCDClearDataAndCommandFlag   ; Monitor-code
                                         ; Monitor-code
OCDGoProgram:
    movw    (6,SP),dw0                   ; Monitor-code
    movw    (4,SP),dw1                   ; Monitor-code
    movw    (2,SP),a0                    ; Monitor-code
    movw    (0,SP),a1                    ; Monitor-code
    addw    8,SP                         ; Monitor-code
    rti                                  ; Monitor-code
OCDMonitorEnd:

#endif
	END
