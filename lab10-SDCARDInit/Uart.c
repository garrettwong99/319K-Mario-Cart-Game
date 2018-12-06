// Uart.c
// Runs on LM4F120/TM4C123
// Use UART1 to implement bidirectional data transfer to and from 
// another microcontroller in Lab 9.  This time, interrupts and FIFOs
// are used.
// Daniel Valvano
// November 14, 2018
// Modified by EE345L students Charlie Gough && Matt Hawk
// Modified by EE345M students Agustinus Darmawan && Mingjie Qiu

/* Lab solution, Do not post
 http://users.ece.utexas.edu/~valvano/
*/

// This U0Rx PC4 (in) is connected to other LaunchPad PC5 (out)
// This U0Tx PC5 (out) is connected to other LaunchPad PC4 (in)
// This ground is connected to other LaunchPad ground

#include <stdint.h>
#include "Fifo.h"
#include "Uart.h"
#include "../inc/tm4c123gh6pm.h"

uint32_t DataLost; 
// Initialize UART1
// Baud rate is 115200 bits/sec
// Make sure to turn ON UART1 Receiver Interrupt (Interrupt 6 in NVIC)
// Write UART1_Handler
void Uart_Init(void){ volatile uint32_t delay;
  SYSCTL_RCGCUART_R |= 0x0002; // activate UART 1
	SYSCTL_RCGCGPIO_R |= 0x00000004;  // activate port C
	delay = SYSCTL_RCGCGPIO_R;
	UART1_CTL_R &= ~0x00000001;    // disable UART
	UART1_IBRD_R = 43;     // IBRD = int(80,000,000/(16*115,200)) = int(43.40278)
	UART1_FBRD_R = 26;     // FBRD = round(0.40278 * 64) = 26
	UART1_LCRH_R = 0x00000070;  // 8 bit, no parity bits, one stop, FIFOs

	
	UART1_IM_R |= 0x010;
	UART1_IFLS_R &= (~0x38);;
	UART1_IFLS_R += 0x08;
  NVIC_PRI1_R &= 0xFF0FFFFF;
	NVIC_PRI1_R += 0x0000;
	NVIC_EN0_R |= 0x40;	
	
	
	UART1_CTL_R |= 0x00000301;    // disable UART
	GPIO_PORTC_AFSEL_R |= 0x30;    // enable alt funct on PC5-4
  GPIO_PORTC_DEN_R |= 0x30;      // configure PC5-4 as UART1
  GPIO_PORTC_PCTL_R = (GPIO_PORTC_PCTL_R&(~0x00FF0000))+0x00220000; // enable UART for PC4 & 5
  GPIO_PORTC_AMSEL_R &= ~0x30;   // disable analog on PC5-4
	//GPIO_PORTC_PUR_R |= 0x20;
	//GPIO_PORTC_DIR_R |= 0x20;
	
}

// input ASCII character from UART
// spin if RxFifo is empty
// Receiver is interrupt driven
char Uart_InChar(void){
  return 0; // --UUU-- remove this, replace with real code
}

//------------UART1_InMessage------------
// Accepts ASCII characters from the serial port
//    and adds them to a string until ETX is typed
//    or until max length of the string is reached.
// Input: pointer to empty buffer of 8 characters
// Output: Null terminated string
// THIS FUNCTION IS OPTIONAL
void UART1_InMessage(char *bufPt){
}


//------------UART1_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
// Transmitter is busywait
void Uart_OutChar(char data){
  while((UART1_FR_R&0x0020) != 0);      // wait until TXFF is 0
  UART1_DR_R = data;
	
}

// hardware RX FIFO goes from 7 to 8 or more items
// UART receiver Interrupt is triggered; This is the ISR
void UART1_Handler(void){
	static char data;
  GPIO_PORTF_DATA_R ^= 0x04; // toggle led.
	GPIO_PORTF_DATA_R ^= 0x04; // toggle led
	while ((UART1_FR_R&0x10)==0){
		data = (char)(UART1_DR_R&0xFF);
		Fifo_Put(data);
	}
	UART1_ICR_R = 0x10;   // this clears bit 4 (RXRIS) in the RIS register
	GPIO_PORTF_DATA_R ^= 0x04;
}
