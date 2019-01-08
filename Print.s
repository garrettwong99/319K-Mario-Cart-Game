; Print.s
; Student names: change this to your names or look very silly
; Last modification date: change this to the last modification date or look very silly
; Runs on LM4F120 or TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; ST7735_OutChar   outputs a single 8-bit ASCII character
; ST7735_OutString outputs a null-terminated string 

    IMPORT   ST7735_OutChar
    IMPORT   ST7735_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix

    AREA    |.text|, CODE, READONLY, ALIGN=2
	PRESERVE8
    THUMB

Stars DCB "*.***"

;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutDec
     PUSH {LR}
     CMP R0, #10           ; character < 10
     BLO End               ; if so, end 
     MOV R2, #10           ; R2 = divisor = 10
     UDIV R3, R0, R2       ; R3 = N/10
     MUL R1, R3, R2        ; R1 = (N/10)*10
     SUB R1, R0, R1        ; R1 = N%10
     PUSH {R1}
     MOVS R0, R3           ; N = N/10
     BL LCD_OutDec         ; LCD_OutDec(N/10)
     POP {R0}
End
     ADD R0, R0, #'0'      ; convert ASCII
     BL ST7735_OutChar     ; print chararacter
     POP {PC}      ; restore and return

      BX  LR
;* * * * * * * * End of LCD_OutDec * * * * * * * *


; Array for string to be outputted
firstDigit EQU 0
period	EQU 1
secondDigit EQU 2
thirdDigit EQU 3
fourthDigit EQU 4
NULLIndex EQU 5
	
countFix EQU 8
divide10 EQU 12


; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.001, range 0.000 to 9.999
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.000 "
;       R0=3,    then output "0.003 "
;       R0=89,   then output "0.089 "
;       R0=123,  then output "0.123 "
;       R0=9999, then output "9.999 "
;       R0>9999, then output "*.*** "
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutFix
	PUSH {R4, LR}
	SUB SP, #16		; Allocate array for output
	
	; Set null terminated array with "0.000"
	MOV R2, #0x2E		; ASCII equivalent of '.'
	STRB R2, [SP, #period]		; Place '.' in second slot of array
	MOV R2, #0x30				
	STRB R2, [SP, #firstDigit]		; Store '0' into array
	STRB R2, [SP, #secondDigit]
	STRB R2, [SP, #thirdDigit]
	STRB R2, [SP, #fourthDigit]
	MOV R2, #0
	STRB R2, [SP, #NULLIndex]		; Add sentinel to end of array
	

	MOV R2, #4
	STR R2, [SP, #countFix]		; Set counter to 4
	MOV R2, #10
	STR R2, [SP, #divide10]
	
; Check if Input (R0) > 9999. If so, output "*.***"
	MOV R1, #9999
	CMP R0, R1		;if R0 > 9999, then output "*.***"
	BLS outputFix
	LDR R0, =Stars		; R0 contains address (ptr) to Stars string
	B outputOnDisplay		; Branch to Output stars onto Display

outputFix
	LDR R12, [SP, #countFix]
continueModulo
	CMP R0, #0
	BEQ outputFixDisplay	; If input (R0) = 0, then output to LCD

	LDR R1, [SP, #divide10]		; R1 contains 10
	
	UDIV R3, R0, R1		; R3 = R0 / 10
	MUL R2, R3, R1		; Multiply answer by 10
	SUB R2, R0, R2		; Subtract to get modulo (remainder)
	
	MOV R0, R3			; R0 now contains R0 / 10 for next iteration
	ADD R2, R2, #0x30		; Convert to ASCII

; Function to store each digit in array
	CMP R12, #4		; Store ones digit if input
	BNE third
	STRB R2, [SP, #fourthDigit]
	SUB R12, #1
	B continueModulo
	
third		; When 10 <= input < 100, then store tens digit
	CMP R12, #3
	BNE second
	STRB R2, [SP, #thirdDigit]
	SUB R12, #1
	B continueModulo

second		; When 100 <= input < 1000, then store hundreds digit
	CMP R12, #2
	BNE first
	STRB R2, [SP, #secondDigit]
	SUB R12, #1
	B continueModulo
	
first			; When 1000 <= input < 9999, then store thouasands digit
	STRB R2, [SP, #firstDigit]
	
outputFixDisplay	;  set input parameter for Fixed Decimal
	ADD R0, SP, #firstDigit		; R0 now points to first element of NULL terminated string array
outputOnDisplay		; Output String to Display
	BL ST7735_OutString		; Output string
	
	ADD SP, #16			; Deallocate
	POP {R4, LR}		; Restore original LR bc funtion was called

	BX   LR

   ALIGN
;* * * * * * * * End of LCD_OutFix * * * * * * * *

     ALIGN                           ; make sure the end of this section is aligned
     END                             ; end of file
