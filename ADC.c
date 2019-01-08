// ADC Library for Mario Kart
// Sihyung Woo, Garrett Wong
//
//		JoyStick ADC Initialization
//		Conversion for ADC to turn amount
//
//		Controller Initialization



#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"



//	ADC Initialization 
//		For controllers (x2) joysticks (Potentiometers) Left and Right only
//		INPUT : None
//		OUTPUT : None
//
void Joystick_ADC_Init(void){ 
	volatile uint32_t delay;
	SYSCTL_RCGCADC_R |= 0x03;		// Activate clock for ADC1 and ADC0
	SYSCTL_RCGCGPIO_R |= 0x08;		// Activate Port D clock
	delay = SYSCTL_RCGCGPIO_R;
	GPIO_PORTD_DIR_R &= (~0x0C);	// Make PD2, PD3 inputs
	GPIO_PORTD_AFSEL_R |= 0x0C;		// Turn on alternate function of PD2 and PD3
	GPIO_PORTD_DEN_R &= (~0x0C);	// Disable Digital for PD2 and PD3
	GPIO_PORTD_AMSEL_R |= 0x0C;		// Turn on analog mode for PD2 and PD3

	// ADC0 Initialization
	ADC0_PC_R = 0x01;					// Set sampling rate at 125k samples/sec
	ADC0_SSPRI_R = 0x0123;				// Set Sequence 3 as highest priority
	ADC0_ACTSS_R &= (~0x08);				// Diasble sampling sequencer 3
	ADC0_EMUX_R &= (~0x0F000);			// Configure trigger event (0 bit at b15-b12)
	ADC0_SSMUX3_R &= ~0x000F;       	// Clear SS3 field
  	ADC0_SSMUX3_R += 5;             	// Set channel Ain5 (PD2)
  	ADC0_SSCTL3_R = 0x06;				// Set sample control bits (IE0, END0)
  	ADC0_IM_R &= (~0x08);				// Disable Interrupts
  	ADC0_ACTSS_R |= 0x08;				// Enable sampling sequencer 3

	/*		For Second Controller
	// ADC1 Initialization
	ADC1_PC_R = 0x01;					// Set sampling rate at 125k samples/sec
	ADC1_SSPRI_R = 0x0123;				// Set Sequence 3 as highest priority
	ADC1_ACTSS_R = (~0x08);				// Disable sampling sequencer 3
	ADC1_EMUX_R &= (~0x0F000);			// Configure trigger event (0 bit at b15-b12)
	ADC1_SSMUX3_R &= ~0x000F;       	// Clear SS3 field
    ADC1_SSMUX3_R += 4;             	// Set channel Ain4 (PD3)
    ADC1_SSCTL3_R = 0x06;				// Set sample control bits (IE0, END0)
    ADC1_IM_R &= (~0x08);				// Disable interrupts
    ADC1_ACTSS_R |= 0x08;				// Enable sampling sequncer 3
	*/
}



//	Conversion for ADC
//		There are 5 angles to where the car leans while turning.		
//		Joystick straight up has a value of 2047 (range: 0 - 4095);
//				0 - 819	Turn Hard (Left/Right : Check Joystick Orientation)
//				820 - 1639 Turn slightly (Left/Right : Check Joystick Orientation)
//				1640 - 2457 Go straight
//				2458 - 3276	Turn slightly (Left/Right : Check Joystick Orientation)
//				3277 - 4095 Turn Hard (Left/Right : Check Joystick Orientation)
//
//		INPUT : ADC value
//		OUTPUT : 0 - go straight
//						 1 - Hard Left
//						 2 - Slight Left
//						 3 - Hard Right
//						 4 - Slight Right
//
uint8_t turnConversion(uint16_t ADCValue) {
	if(ADCValue > 1639 && ADCValue <= 2457) {		// if ADC between 1640 - 2457, go straight (return 0)
		return 0;
	} else if(ADCValue > 0 && ADCValue <= 819) {		// if ADC between 0 - 819, hard left
		return 1;
	} else if(ADCValue > 819 && ADCValue <= 1639) {		// if ADC between 820 - 1639, slight left
		return 2;
	} else if(ADCValue > 2457 && ADCValue <= 3276) {		// if ADC between 2458 - 3276, slight right
		return 3;
	} else if(ADCValue > 3276 && ADCValue <= 4095) {		// if ADC between 3277 - 4095, hard right
		return 4;
	}
	return 0;				// if none of the conditions are met for some odd reason, don't turn.
}


// Acceleration_In
//		Retrieve button input for acceleration
//		PC0 is accelerate button pin
//
//	INPUT : none
// 	OUTPUT : 1 if button pressed (accelerates)
//				 : 0 if not pressed
//
uint8_t Acceleration_In(void) {
	if((GPIO_PORTC_DATA_R & 0x01) == 1) {		// if pressed, then return 1
		return 1;
	}
	else {						// if not pressed, then return 0
		return 0;
	}
}

// Brake_In
//		Retrieve button input for brake
//		PC1 is brake button pin
//
//	INPUT : none
// 	OUTPUT : 1 if button pressed (brakes)
//				 : 0 if not pressed
//
uint8_t Brake_In(void) {
	if((GPIO_PORTC_DATA_R & 0x02) == 1) {		// if pressed, then return 1
		return 1;
	}
	else {						// if not pressed, then return 0
		return 0;
	}
}








//	Controllers Initialization
//		Initializes Joysticks and buttons for controllers
//		INPUT : None
//		OUTPUT : None
//
void Controllers_Init(void) {
	uint32_t volatile delay;
	SYSCTL_RCGCGPIO_R |= 0x04;		// Turn on clocks for Port C and D
	delay = SYSCTL_RCGCGPIO_R;
	GPIO_PORTC_DIR_R &= (~0x0F);		// PC0-PC3 are now inputs
	GPIO_PORTC_DEN_R |= 0x0F;		// Digital enable PC0-PC3

	Joystick_ADC_Init();			// Initialize joysticks

}
