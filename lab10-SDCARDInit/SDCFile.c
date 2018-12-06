// SDCFile.c
// Runs on TM4C123
// This program is a simple demonstration of the SD card,
// file system, and ST7735 LCD.  It will read from a file,
// print some of the contents to the LCD, and write to a
// file.
// Daniel Valvano
// January 13, 2015

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2014
   Program 4.6, Section 4.3
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014
   Program 2.10, Figure 2.37

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
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

// hardware connections
// **********ST7735 TFT and SDC*******************
// ST7735
// 1  ground
// 2  Vcc +3.3V
// 3  PA7 TFT reset
// 4  PA6 TFT data/command
// 5  PD7 SDC_CS, active low to enable SDC
// 6  PA3 TFT_CS, active low to enable TFT
// 7  PA5 MOSI SPI data from microcontroller to TFT or SDC
// 8  PA2 Sclk SPI clock from microcontroller to TFT or SDC
// 9  PA4 MISO SPI data from SDC to microcontroller
// 10 Light, backlight connected to +3.3 V


#include <stdint.h>
#include "diskio.h"
#include "ff.h"
#include "PLL.h"
#include "ST7735.h"
#include "../inc/tm4c123gh6pm.h"
#include "Sound.h"
#include "ADC.h"
#include "string.h"
#include <stdio.h>
#include "Timer0.h"
#include "Timer1.h"
#include "Uart.h"
#include "Fifo.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay100ms(uint32_t count); // time delay in 0.1 seconds

uint8_t setReadFlag = 0;

void PortF_init(void){						// initialize port f
	GPIO_PORTF_DIR_R |= 0x04;
	GPIO_PORTF_DEN_R |= 0x04;
}

void PortF2_init(void){						// initialize port f for onboard switches
	uint32_t volatile nop;
	SYSCTL_RCGCGPIO_R |= 0x20;
	nop = SYSCTL_RCGCGPIO_R; 
	GPIO_PORTF_LOCK_R = 0x4C4F434B;
	GPIO_PORTF_CR_R = 0x1F;
	GPIO_PORTF_DIR_R =(~0x11);
	GPIO_PORTF_PUR_R = 0x11;
	GPIO_PORTF_DEN_R = 0x11;
}

void PortD_init(void){
	uint32_t volatile nop;
	SYSCTL_RCGCGPIO_R |= 0x08;
  nop = SYSCTL_RCGCGPIO_R;
	GPIO_PORTD_DIR_R &= 0xF0;
	GPIO_PORTD_DEN_R |=0x0F;
}

static FATFS g_sFatFs;
FIL Handle,Handle2;
FRESULT MountFresult;
FRESULT Fresult;
#define MAXBLOCKS 100


/*char OriginalMessage;
void sendUartMessage (char message){
		if(message!=OriginalMessage){
			Uart_OutChar(0x02);
			Uart_OutChar(message+0x30);
			Uart_OutChar(message+0x30);
			Uart_OutChar(0x03);
			OriginalMessage=message;
		}
}
char testbuff[4];
char newmessage;
char newMessageFlag = 0;
void receiveUartMessage(void){
		uint8_t i;
		for(i=0;i<4;i++){
			Fifo_Get(&testbuff[i]);
		}
		if ((testbuff[0]==0x02)&(testbuff[3]==0x03)){
			newMessageFlag = 1;
			newmessage = testbuff[1];
		}			
}

void checkNewMessage (void){
		receiveUartMessage();
		if (newMessageFlag != 0){
			newMessageFlag = 0;
			finishFlag = 1;
			DisableInterrupts();
		}
		
}*/

// function convert number
//this function converts the ascii from the read function to phsyical numbers
void convertnumber (char *pt, char buff[]){	
	DisableInterrupts();
	char *token;
	token = pt;
	if(*token==','){token++;}
	if(*(token+1)==','){token=token+2;}
	uint16_t i;		
	for(i=0;i<=(bufOut-1);i++){
		buff[i] = (((*token-0x30)<<4)/16)*10+(*(token+1)-0x30);
		if (buff[i] == 0x5B){finishFlag=1;DisableInterrupts();}
		token = token +3;
	}	
}
//end of convertnumber


uint32_t enginecounter = 0;
//this function adds a portion of the engine buffer to the buffer 2 music and buffer 3 music, while remembering where it left off
void engineadd (char buff[]){
	uint32_t j;
	for(j=0;j<85;j++){
		if (enginecounter ==11036){enginecounter = 0;}
		buff[j]=((buff[j]+engine2[enginecounter])/2);	
		enginecounter++;
	}
}

//this function checks the engine sound button
//void checkengine (void){
			//if((GPIO_PORTF_DATA_R&1)== 0){TIMER0_CTL_R = 0x00000001;}
			//if((GPIO_PORTF_DATA_R&1)!= 0){TIMER0_CTL_R = 0x00000000;}
//}


//this function checks if a button was pressed. if so exit the song, by finish flag
void checkbutton(uint32_t *pin, uint32_t pinnum){
	if (((*pin>>pinnum)&0x01)==1){
		finishFlag=1;
		DisableInterrupts();
	while(((*pin>>pinnum)&0x01)==1){}
		EnableInterrupts();
	}
}

/*void checkNewMessage(void);
	if(NewMessageFlag == 1){
		finishFlag = 1;
		DisableInterrupts
		
	}*/


//this function plays the small buffer section of the song after it reads from the sd card
//input: *buff , *buffer2 music
UINT successfulreads;								//counter to where to read again
void playsection( char *buffer, char buffmusic[]){
	while((count<(bufOut-1))&(finishFlag==0)){
		EnableInterrupts();
		if(setReadFlag == 0){
					DisableInterrupts();
					//if((GPIO_PORTF_DATA_R&1)== 0){engineadd(buffmusic);}
					f_read(&Handle,buffer,bufSize,&successfulreads);
					EnableInterrupts();
					convertnumber(buffer,buffmusic);
					if((GPIO_PORTD_DATA_R&0x08)== 1){engineadd(buffmusic);}
					setReadFlag += 1;
			}
		}
			count = 0;
			setReadFlag = 0;
			bufferFlag = 0;
			if (buffmusic == buffer3music){bufferFlag=1;}
}

char testbuff[4];
char newmessage;
char newMessageFlag = 0;

void receiveUartMessage(void){
		uint8_t i;
		for(i=0;i<3;i++){
			Fifo_Get(&testbuff[i]);
}		
}
char prevMessage = 0;

void isMessageValid (void){
	if ((testbuff[0]==0x02)&(testbuff[2]==0x03)){
		if((testbuff[1]-0x30)!= message){
			message = (testbuff[1]-0x30);
			finishFlag = 1;
		}
	}
}
	
//this function plays a song given a file to read
void playsong (const TCHAR* path){	
	f_open(&Handle, path, FA_READ);					// open the file to be read
	f_read(&Handle,&buffer2,bufSize,&successfulreads);// reads the first part of the song before double buffer			
	convertnumber(buffer2,buffer2music);
	if((GPIO_PORTF_DATA_R&1)== 0){engineadd(buffer2music);}
	finishFlag=0;	
	
	while(finishFlag==0){
		receiveUartMessage();//get inputs from mcu1												
		isMessageValid();//compare messages//if theyre different set finish Flag to one
		
		EnableInterrupts();
		playsection( buffer2, buffer2music);
		playsection( buffer3, buffer3music);
		//checkbutton(pin,pinnum);
	} 
	f_close(&Handle);
}
//end of playsong


const char CircuitOne[]= "cir_one.txt";
//const char ChooseDriver[]= "driver.txt";
const char FinalLap[]="finlap.txt";
//const char StartRace[]="light.txt";
//const char MarioWins[]="marwin.txt";
const char RainbowRoad[]="rr.txt";
const char MarioKartTheme[]= "theme.txt";
const char TakeOnMe[]= "meme.txt";
//const char ToadWins[]="toadwin";

uint8_t MarioKart_Theme = 0;
uint8_t Circuit_One = 1;
uint8_t Rainbow_Road = 2;
uint8_t Take_On_Me = 3;
//uint8_t Start_Race= 2;
//uint8_t Final_Lap= 2;

//uint8_t Mario_Wins =4;

//uint8_t Choose_Driver= 1;
//uint8_t Toad_Wins =7;

void songselect(uint8_t choose){
	if (choose==Circuit_One){playsong(CircuitOne);}
	//if (choose==Choose_Driver){playsong(ChooseDriver);}
	//if (choose==Final_Lap){playsong(FinalLap);}
	//if (choose==Start_Race){playsong(StartRace);}
	//if (choose==Mario_Wins){playsong(MarioWins);}
	if (choose==Rainbow_Road){playsong(RainbowRoad);}
	if (choose==MarioKart_Theme){playsong(MarioKartTheme);}
	if (choose==Take_On_Me){playsong(TakeOnMe);}
	//if (choose==Toad_Wins){playsong(ToadWins);}	
}

void Init (void){
 DisableInterrupts();
	PLL_Init(Bus80MHz);    							// 80 MHz
  ST7735_InitR(INITR_REDTAB);					// initialize the SSI
  ST7735_FillScreen(0);                 // set screen to black
	Sound_Init();													// initialize dac 
	PortF2_init();												//portF init	
	PortF_init();
	PortD_init();
	Uart_Init();
	Fifo_Init();
	//Timer1_Init(1333333);
	
	Sound_Play(8000);	
}



int main(void){
	Init();
	f_mount(&g_sFatFs, "", 0);												//mount the sd card
	EnableInterrupts();	
	
	//if ((GPIO_PORTF_DATA_R & 0x10)==0){message = 0x30;}
	//if ((GPIO_PORTF_DATA_R & 0x01)==0){message = 0x31;}
	//songselect(message);
	while(1){
	songselect(message);
	}
		/*songselect(Choose_Driver,&GPIO_PORTD_DATA_R,0);
		songselect(Circuit_One,&GPIO_PORTD_DATA_R,0);
		songselect(Final_Lap,&GPIO_PORTD_DATA_R,0);
		songselect(Start_Race,&GPIO_PORTD_DATA_R,0);	
		songselect(Mario_Wins,&GPIO_PORTD_DATA_R,0);
		songselect(Rainbow_Road,&GPIO_PORTD_DATA_R,0);
		songselect(MarioKart_Theme,&GPIO_PORTD_DATA_R,0);		
	}*/
}
