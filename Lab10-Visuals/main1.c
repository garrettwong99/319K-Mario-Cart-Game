// Mario Kart MCU 1
// Runs on LM4F120/TM4C123
// Sihyung Woo and Garrett Wong
//
//		Linear Transformations from OneLoneCoder
//		
/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2018

   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2018

 Copyright 2018 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdint.h>
#include <math.h>
#include "../inc/tm4c123gh6pm.h"
#include "ST7735.h"
#include "Random.h"
#include "PLL.h"
#include "ADC.h"
#include "Images.h"



// Trial By Error Presets. Change Preset Parameters if necessary
float WorldX = 0.0f;								// X coordinate of Map
float WorldY = 0.05f;								// Y coordinate of Map
float WorldA = 45.0f;									// Angle at which you're looking
float Near = 0.001f;									// Frustum closest length from camera
float Far = 0.03f;										// Frustum furthest length from camera
float FoVHalf = 3.14159f / 4.0f;			// Field of View Angle Preset

uint16_t *sym;
uint16_t color;

uint8_t screenVerticalLength = 128;					// LCD is 160x128 pixels
uint8_t screenHorizontalLength = 160;				


void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay100ms(uint32_t count); // time delay in 0.1 seconds

// Our Added Functions:
void SysTick_Init(void);		// Initializes SysTick with Interrupt Polls game controllers
void UserFrame_Update(void);
short SampleGlyph(float x, float y);
short SampleColor(float x, float y);


int main(void){
  PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
  Random_Init(1);


	// Controller Initialization
	// ST7735 Initialization
	// UART/Interrupt Initialization
	// SysTick/Interrupt Initalization
	// Timer Initialization for ADC
	
	ST7735_InitR(INITR_REDTAB);
	ST7735_SetRotation(3);

	//ST7735_DrawBitmap(0, 140, rrbitmap, 123, 123);
	UserFrame_Update();
  while(1){
		// Update LCD
		//UserFrame_Update();
		//ST7735_DrawBitmap(64, 80, toad, 40, 40);
		//	UserFrame_Update();
  }

}


//	UserFrame_Update
//		if controller pressed anything, move the camera angle.
//		then update the frame by :
//				Finding Frustum Points
//				Interpolate Coordinates for every pixel then place on screen
//
//	Math Explanation : 
//
//
void UserFrame_Update(void) {
	uint16_t x;		// Don't know length of bitmap array
	uint16_t y;		
	
	float FarX1 = WorldX + cosf(WorldA - FoVHalf) * Far;			// Get the Left Further X point of Frustum
	float FarY1 = WorldY + sinf(WorldA - FoVHalf) * Far;			// Get the Left Further Y point of Frustum
	
	float NearX1 = WorldX + cosf(WorldA - FoVHalf) * Near;		// Get the Left Closer X point of Frustum
	float NearY1 = WorldY + sinf(WorldA - FoVHalf) * Near;		// Get the Left Closer Y point of Frustum
	
	float FarX2 = WorldX + cosf(WorldA + FoVHalf) * Far;			// Get the Right Further X point of Frustum
	float FarY2 = WorldY + sinf(WorldA + FoVHalf) * Far;			// Get the Right Further Y point of Frustum
	
	float NearX2 = WorldX + cosf(WorldA + FoVHalf) * Near;		// Get the Right Closer X point of Frustum
	float NearY2 = WorldX + sinf(WorldA + FoVHalf) * Near;		// Get the Right Closer Y point of Frustum
	
	// Calculates all pixel scaling and orientation of map then displays
	//		Starts at furthest frustum length to closest frustum length from camera
	//		Row then Columns. Go to each row and fill in the pixels with 2 loops
	for(y = 0; y < (screenVerticalLength / 2); y++) {
		float sampleDepth = (float)y / ((float)screenVerticalLength / 2.0f);		// Depth Parameter of screen
		
		// Perspective from 2D x projection to y axis is a 1/x relationship
		// Get Bounds of X and Y rows you are filling
		float StartX = (FarX1 - NearX1) / (sampleDepth) + NearX1;			// Start Bound for X
		float StartY = (FarY1 - NearY1) / (sampleDepth) + NearY1;			// Start Bound for Y
		
		float EndX = (FarX2 - NearX2) / (sampleDepth) + NearX2;				// End Bound for X
		float EndY = (FarY2 - NearY2) / (sampleDepth) + NearY2;				// End Bound for Y
			
		// Fill out the columns on current Y pixel row
		for(x = 0; x < (screenHorizontalLength); x++) {
			float sampleWidth = (float)x / (float)screenHorizontalLength;
			
			// Calculate current x coordinate with offset
			float currentX = (EndX - StartX) * sampleWidth + StartX;			
			float currentY = (EndY - StartY) * sampleWidth + StartY;

			// Update Frame
			color = SampleColor(currentX, currentY);
			ST7735_DrawPixel(x, y + (screenVerticalLength / 2), color);
		}

		
	}
	
	
}


		// width = width of array
		// height of array
short SampleGlyph(float x, float y) {
		int sx = (int)(x * (float)123);
		int sy = (int)(y * (float)123-1.0f);
		if (sx <0 || sx >= 123 || sy < 0 || sy >= 123)
			return 0;
		else
			return rrbitmap[sy * 123 + sx];
}

	
short SampleColor(float x, float y){
		int sx = (int)(x * (float)123);
		int sy = (int)(y * (float)123-1.0f);
		if (sx <0 || sx >= 123 || sy < 0 || sy >= 123)
			return 0x0;
		else
			return rrbitmap[sy * 123 + sx];
}









//	SysTick Initialization
//			Period currently set at 16.67 ms
//			Priority = 4 CHANGE IF NEED TO
//	INPUT : none 
//	OUTPUT : none
void SysTick_Init(void) {
	NVIC_ST_CTRL_R = 0x00;		// Disable
	NVIC_ST_RELOAD_R = 1333600 - 1;	// Delay set to 16.67 ms = 0.01667 s
	NVIC_ST_CURRENT_R = 0;		// Clear current value
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x80000000;		// Set priority to 4
	NVIC_ST_CTRL_R = 0x07;		// Enable interrupt and clock;
}









//	SysTick ISR
//		Polls the control inputs and places them in mailboxes?????
//
/*
void SysTick_Handler(void) {
	// Sample ADC
	direction = turnConversion(data);
	// Check for Joystick input
	switch (direction) {
		case 0:			// if 0, go straight, don't turn
			break;
		case 1:			// if 1, hard left
			WorldA -= 1.0f;
			break;
		case 2:			// if 2, slight left
			WorldA -= 0.5f;
			break;
		case 3:			// if 3, slight right
			WorldA += 0.5f;
			break;
		case 4:			// if 4, hard right
			WorldA += 1.0f;
			break;
	}
	
	// Go forward if button is pressed
	accelerate = Accelerate_In();
	if(accelerate == 1) {
		WorldX += cosf(WorldA) * 0.02f;
		WorldY += sinf(WorldA) * 0.02f;
	}
	
	// If brake pressed, then reverse or stop
	brake = Brake_In();
	if(brake == 1) {
		WorldX -+ cosf(WorldA) * 0.02f;
		WorldY -= sinf(WorldA) * 0.02f;
	}

}
*/

