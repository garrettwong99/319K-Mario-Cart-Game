// put implementations for functions, explain how it works
// put your names here, date

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

void DAC_Init(void){unsigned long volatile delay;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB; 			// activate port B								//initialize digital outputs
	delay = SYSCTL_RCGC2_R;    									// allow time to finish activating
	GPIO_PORTB_AMSEL_R &= ~0x07;     						// no analog
	GPIO_PORTB_PCTL_R &= ~0x00000FFF; 					// regular GPIO function
	GPIO_PORTB_DIR_R |= 0x3F;										// make PB0-5 an out
	GPIO_PORTB_AFSEL_R &= ~0x3F;   							// disable alt funct on PB0-5
	GPIO_PORTB_DEN_R |= 0x3F;			 							// emable digital I/O on PB0-5
}

void DAC_Out(uint16_t data){
	GPIO_PORTB_DATA_R = data;
}
