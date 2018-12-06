// Sound.h
// Runs on TM4C123 or LM4F120
// Prototypes for basic functions to play sounds from the
// original Space Invaders.
// Jonathan Valvano
// November 17, 2014
#include "ff.h"

#define bufSize 256
#define bufOut 85

extern unsigned char engine2[11036];
extern char buffer2[bufSize];
extern char buffer3[bufSize];
extern char buffer2music[bufOut];
extern char buffer3music[bufOut];
extern uint16_t count;
extern uint8_t bufferFlag;
extern uint8_t finishFlag;

void Sound_Init(void);
void Sound_Play(uint32_t count);
void Sound_Shoot(void);
void Sound_Killed(void);
void Sound_Explosion(void);

void Sound_Fastinvader1(void);
void Sound_Fastinvader2(void);
void Sound_Fastinvader3(void);
void Sound_Fastinvader4(void);
void Sound_Highpitch(void);

