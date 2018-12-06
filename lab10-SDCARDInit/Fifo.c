// Fifo.c
// Runs on LM4F120/TM4C123
// Provide functions that implement the Software FiFo Buffer
// Last Modified: November 14, 2018
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly

#include <stdint.h>
// --UUU-- Declare state variables for Fifo
//        buffer, put and get indexes

#define size 4
#define win 1
#define loss 0

uint8_t static PutI; // put next
uint8_t static GetI; // get next
uint8_t  Fifo[size];

// *********** Fifo_Init**********
// Initializes a softwar  e FIFO of a
// fixed size and sets up indexes for
// put and get operations
void Fifo_Init() {
	GetI = PutI = 0; // places both pointers at the first adress of the array
}

// *********** Fifo_Put**********
// Adds an element to the FIFO
// Input: Character to be inserted
// Output: 1 for success and 0 for failure
//         failure is when the buffer is full
uint32_t Fifo_Put(char data) {
	if ((PutI+1)%size == GetI){return(0);}
	Fifo[PutI] = data;
	PutI = (PutI+1) % size;
	return(1);
}

// *********** FiFo_Get**********
// Gets an element from the FIFO
// Input: Pointer to a character that will get the character read from the buffer
// Output: 1 for success and 0 for failure
//         failure is when the buffer is empty
uint32_t Fifo_Get(char *datapt)
{
	if(GetI== PutI){return(0);}
	*datapt = Fifo[GetI];
	GetI = (GetI + 1)%size;
	return(1);
}



