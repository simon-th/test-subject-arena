// dac.c
// This software configures DAC output
// Runs on LM4F120 or TM4C123
// Program written by: Simon Hoque and Shania Paul
// Date Created: 3/6/17 
// Last Modified: 4/28/18 

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data

// **************DAC_Init*********************
// Initialize 4-bit DAC, called once 
// Input: none
// Output: none
void DAC_Init(void){
	uint8_t delay = 0;
	SYSCTL_RCGCGPIO_R |= 0x2;
	delay ++;
	delay ++;
	delay ++;
	delay ++;
	delay ++;
	delay ++;
	GPIO_PORTB_DIR_R |= 0xF;
	GPIO_PORTB_AFSEL_R = 0;
	GPIO_PORTB_AMSEL_R = 0;
	GPIO_PORTB_DEN_R |= 0xF;
}

// **************DAC_Out*********************
// output to DAC
// Input: 4-bit data, 0 to 15
// Input=n is converted to n*3.3V/15
// Output: none
void DAC_Out(uint32_t data){
	GPIO_PORTB_DATA_R = data & 0xF;
}// put implementations for functions, explain how it works
// put your names here, date
