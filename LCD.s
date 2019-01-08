; LCD.s
; Student names: change this to your names or look very silly
; Last modification date: change this to the last modification date or look very silly

; Runs on LM4F120/TM4C123
; Use SSI0 to send an 8-bit code to the ST7735 160x128 pixel LCD.

; As part of Lab 7, students need to implement these LCD_WriteCommand and LCD_WriteData
; This driver assumes two low-level LCD functions

; Backlight (pin 10) connected to +3.3 V
; MISO (pin 9) unconnected
; SCK (pin 8) connected to PA2 (SSI0Clk)
; MOSI (pin 7) connected to PA5 (SSI0Tx)
; TFT_CS (pin 6) connected to PA3 (SSI0Fss)
; CARD_CS (pin 5) unconnected
; Data/Command (pin 4) connected to PA6 (GPIO)
; RESET (pin 3) connected to PA7 (GPIO)
; VCC (pin 2) connected to +3.3 V
; Gnd (pin 1) connected to ground

DC                      EQU   0x40004100
DC_COMMAND              EQU   0
DC_DATA                 EQU   0x40
SSI0_DR_R               EQU   0x40008008
SSI0_SR_R               EQU   0x4000800C
SSI_SR_RNE              EQU   0x00000004  ; SSI Receive FIFO Not Empty
SSI_SR_BSY              EQU   0x00000010  ; SSI Busy Bit
SSI_SR_TNF              EQU   0x00000002  ; SSI Transmit FIFO Not Full

      EXPORT   writecommand
      EXPORT   writedata

      AREA    |.text|, CODE, READONLY, ALIGN=2
      THUMB
      ALIGN

; The Data/Command pin must be valid when the eighth bit is
; sent.  The SSI module has hardware input and output FIFOs
; that are 8 locations deep.  Based on the observation that
; the LCD interface tends to send a few commands and then a
; lot of data, the FIFOs are not used when writing
; commands, and they are used when writing data.  This
; ensures that the Data/Command pin status matches the byte
; that is actually being transmitted.
; The write command operation waits until all data has been
; sent, configures the Data/Command pin for commands, sends
; the command, and then waits for the transmission to
; finish.
; The write data operation waits until there is room in the
; transmit FIFO, configures the Data/Command pin for data,
; and then adds the data to the transmit FIFO.
; NOTE: These functions will crash or stall indefinitely if
; the SSI0 module is not initialized and enabled.

; This is a helper function that sends an 8-bit command to the LCD.
; Input: R0  8-bit command to transmit
; Output: none
; Assumes: SSI0 and port A have already been initialized and enabled
writecommand
;1) Read SSI0_SR_R and check bit 4, 
;2) If bit 4 is high, loop back to step 1 (wait for BUSY bit to be low)
;3) Clear D/C=PA6 to zero
;4) Write the command to SSI0_DR_R
;5) Read SSI0_SR_R and check bit 4, 
;6) If bit 4 is high, loop back to step 5 (wait for BUSY bit to be low)

    LDR R1, =SSI0_SR_R              ;    R1 = &SSI0_SR_R
    LDR R3, =SSI_SR_BSY             ;    R3 = SSI_SR_BSY
LCDwcloop1                          ; 1) return here while SSI0 busy/transmit FIFO not empty
    LDR R2, [R1]                    ; 1) read SSI0_SR_R
    AND R2, R2, R3                  ; 2) check SSI_SR_BSY
    CMP R2, R3                      ; 2) is SSI0_SR_R&SSI_SR_BSY == SSI_SR_BSY?
    BEQ LCDwcloop1                  ; 2) if so, keep polling
    LDR R1, =DC                     ; 3) R1 = &DC
    MOV R2, #DC_COMMAND             ; 3) R2 = DC_COMMAND
    STR R2, [R1]                    ; 3) [R1] = R2 (DC pin configured for command)
    LDR R1, =SSI0_DR_R              ; 4) R1 = &SSI0_DR_R
    STR R0, [R1]                    ; 4) [R1] = R0 (store message in SSI0 Data Register)
    LDR R1, =SSI0_SR_R              ;     R1 = &SSI0_SR_R
LCDwcloop2                          ; 5) return here while SSI0 busy/transmit FIFO not empty
    LDR R2, [R1]                    ; 5) R2 = [R1]
    AND R2, R2, R3                  ; 6) R2 = R2&R3 = SSI0_SR_R&SSI_SR_BSY
    CMP R2, R3                      ; 6) is SSI0_SR_R&SSI_SR_BSY == SSI_SR_BSY?
    BEQ LCDwcloop2                  ; 6 if so, keep polling
    
    BX  LR                          ;   return

; This is a helper function that sends an 8-bit data to the LCD.
; Input: R0  8-bit data to transmit
; Output: none
; Assumes: SSI0 and port A have already been initialized and enabled
writedata
;1) Read SSI0_SR_R and check bit 1, 
;2) If bit 1 is low loop back to step 1 (wait for TNF bit to be high)
;3) Set D/C=PA6 to one
;4) Write the 8-bit data to SSI0_DR_R

    LDR R1, =SSI0_SR_R              ;    R1 = &SSI0_SR_R
    LDR R3, =SSI_SR_TNF             ;    R3 = SSI_SR_TNF
LCDwdloop1                          ; 1) return here while SSI0 transmit FIFO full
    LDR R2, [R1]                    ; 1) read SSI0_SR_R
    ANDS R2, R2, R3                 ; 2) check SSI_SR_TNF
                                    ; 2) TNF is 0 if the FIFO is full
    BEQ LCDwdloop1                  ; 2) keep polling if transmit FIFO is not not_full
    LDR R1, =DC                     ; 3) R1 = &DC
    MOV R2, #DC_DATA                ; 3) R2 = DC_DATA
    STR R2, [R1]                    ; 3) Configure for data
    LDR R1, =SSI0_DR_R              ; 4) &SSI0_DR_R
    STR R0, [R1]                    ; 4) store message in SSI0 Data Register)
    
    BX  LR                          ;   return


;***************************************************
; This is a library for the Adafruit 1.8" SPI display.
; This library works with the Adafruit 1.8" TFT Breakout w/SD card
; ----> http://www.adafruit.com/products/358
; as well as Adafruit raw 1.8" TFT display
; ----> http://www.adafruit.com/products/618
;
; Check out the links above for our tutorials and wiring diagrams
; These displays use SPI to communicate, 4 or 5 pins are required to
; interface (RST is optional)
; Adafruit invests time and resources providing this open source code,
; please support Adafruit and open-source hardware by purchasing
; products from Adafruit!
;
; Written by Limor Fried/Ladyada for Adafruit Industries.
; MIT license, all text above must be included in any redistribution
;****************************************************

    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file