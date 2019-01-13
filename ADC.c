// ADC.c
// Runs on LM4F120/TM4C123
// Provide functions that initialize ADC0
// Last Modified: 4/23/2018 
// Student names: Shania Paul and Simon Hoque

#include <stdint.h>
#include "../../EE319KwareSpring2018/inc/tm4c123gh6pm.h"

uint32_t ADCvalue[4];

// ADC initialization function 
// Input: none
// Output: none
// measures from PD2, analog channel 5
void ADC_Init(void){
	uint8_t delay = 0;
  SYSCTL_RCGCGPIO_R |= 0x8;      // 1) activate clock for Port D
  delay ++;
	delay ++;
	delay ++;
	delay ++;
  GPIO_PORTD_DIR_R &= ~0xF;      // 2) make PD0-3 input
  GPIO_PORTD_AFSEL_R |= 0xF;     // 3) enable alternate function on PD0-3
  GPIO_PORTD_DEN_R &= ~0xF;      // 4) disable digital I/O on PD0-3
  GPIO_PORTD_AMSEL_R |= 0xF;    // 5) enable analog function on PD0-3
  SYSCTL_RCGCADC_R |= 0x01;       // 6) activate ADC0
	for (uint8_t i = 0; i < 50; i ++)	{
		delay ++;
	}
	ADC0_PC_R &= ~0xF;              // 8) clear max sample rate field
  ADC0_PC_R |= 0x1;               //    configure for 125K samples/sec
  ADC0_SSPRI_R = 0x3210;          // 9) Sequencer 3 is lowest priority
  ADC0_ACTSS_R &= ~0x0004;        // 10) disable sample sequencer 2
  ADC0_EMUX_R &= ~0x0F00;         // 11) seq2 is software trigger
  ADC0_SSMUX2_R = 0x7654;         // 12) set channels for SS2 (PD0-3)
  ADC0_SSCTL2_R = 0x6000;         // 13) no TS0 D0 IE0 END0 TS3 D3, yes IE3 END3
  ADC0_IM_R &= ~0x0004;           // 14) disable SS2 interrupts
  ADC0_ACTSS_R |= 0x0004;         // 15) enable sample sequencer 2
}


//------------ADC_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: two 12-bit result of ADC conversions
// Samples ADC4 and ADC5 
// 125k max sampling
// software trigger, busy-wait sampling
// data returned by reference
// data[3] is ADC4 (PD3) 0 to 4095
// data[2] is ADC5 (PD2) 0 to 4095
// data[1] is ADC6 (PD1) 0 to 4095
// data[0] is ADC7 (PD0) 0 to 4095

void ADC_In(uint32_t data[4]){  
  ADC0_PSSI_R = 0x0004;            // 1) initiate SS2
  while((ADC0_RIS_R&0x04)==0){};   // 2) wait for conversion done
  data[3] = ADC0_SSFIFO2_R&0xFFF;  // 3A) read first result - PD3
  data[2] = ADC0_SSFIFO2_R&0xFFF;  // 3B) read second result - PD2
	data[1] = ADC0_SSFIFO2_R&0xFFF;  // 3B) read third result - PD1
	data[0] = ADC0_SSFIFO2_R&0xFFF;  // 3B) read fourth result - PD0
  ADC0_ISC_R = 0x0004;             // 4) acknowledge completion
}

