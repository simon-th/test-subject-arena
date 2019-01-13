// This file contains initialization functions for the buttons and slide potentiometer.

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "ADC.h"

void (*PeriodicTask0)(void);
void (*PeriodicTask1)(void);
void (*PeriodicTask2)(void);
void (*PeriodicTask3)(void);

// This function initializes Ports A and B which will be interfaced for the buttons.
// Player 1 buttons: PA2-3 ; input
// Player 2 buttons: PB0-1 ; input

void Button_Init (void) {
	uint32_t delay;
	SYSCTL_RCGCGPIO_R |= 0x3F;				// enable clock on all ports
//	SYSCTL_RCGC2_R |= 0x03;
	delay ++;													// delay to stabilize clock
	delay ++;
	delay ++;
	delay ++;
	delay ++;
	delay ++;
	
	GPIO_PORTE_AMSEL_R = 0x00;        // disable analog on PE3
  GPIO_PORTE_DIR_R &= ~0x8;          // PE0-3 in
  GPIO_PORTE_AFSEL_R = 0x00;        // disable alt funct on PE3
  GPIO_PORTE_DEN_R |= 0x8;						// enable IO data on PE3
	
	GPIO_PORTC_AMSEL_R = 0x00;        // disable analog on PC
  GPIO_PORTC_DIR_R &= ~0x10;          // PC4 in
  GPIO_PORTC_AFSEL_R = 0x00;        // disable alt funct on PC
  GPIO_PORTC_DEN_R |= 0x10;						// enable IO data on PC4
	
	GPIO_PORTB_AMSEL_R = 0x00;        // disable analog on PB
  GPIO_PORTB_DIR_R &= ~0x10;          // PB4 in
  GPIO_PORTB_AFSEL_R = 0x00;        // disable alt funct on PB
  GPIO_PORTB_DEN_R |= 0x10;						// enable IO data on PB4
	
	GPIO_PORTA_AMSEL_R = 0x00;        // disable analog on PA
  GPIO_PORTA_DIR_R &= ~0x10;          // PA4 in
  GPIO_PORTA_AFSEL_R = 0x00;        // disable alt funct on PA
  GPIO_PORTA_DEN_R |= 0x10;						// enable IO data on PA4
	
//	SYSCTL_RCGC2_R |= 0x00000020;     // 1) activate clock for Port F

  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog on PF
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PF4-0
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 in, PF3-1 out
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) disable alt funct on PF7-0
  GPIO_PORTF_PUR_R = 0x11;          // enable pull-up on PF0 and PF4
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital I/O on PF4-0
}

void Timer0_Init(void(*task)(void), uint32_t period){
  SYSCTL_RCGCTIMER_R |= 0x01;   // 0) activate TIMER0
  PeriodicTask0 = task;          // user function
  TIMER0_CTL_R = 0x00000000;    // 1) disable TIMER0A during setup
  TIMER0_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER0_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER0_TAILR_R = period-1;    // 4) reload value
  TIMER0_TAPR_R = 0;            // 5) bus clock resolution
  TIMER0_ICR_R = 0x00000001;    // 6) clear TIMER0A timeout flag
  TIMER0_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x80000000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 35, interrupt number 19
  NVIC_EN0_R = 1<<19;           // 9) enable IRQ 19 in NVIC
  TIMER0_CTL_R = 0x00000001;    // 10) enable TIMER0A
}

void Timer0A_Handler(void){
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER0A timeout
  (*PeriodicTask0)();                // execute user task
}

void Timer1_Init(void(*task)(void), uint32_t period){
  SYSCTL_RCGCTIMER_R |= 0x02;   // 0) activate TIMER1
  PeriodicTask1 = task;          // user function
  TIMER1_CTL_R = 0x00000000;    // 1) disable TIMER1A during setup
  TIMER1_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER1_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER1_TAILR_R = period-1;    // 4) reload value
  TIMER1_TAPR_R = 0;            // 5) bus clock resolution
  TIMER1_ICR_R = 0x00000001;    // 6) clear TIMER1A timeout flag
  TIMER1_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI5_R = (NVIC_PRI5_R&0xFFFF00FF)|0x00008000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 37, interrupt number 21
  NVIC_EN0_R = 1<<21;           // 9) enable IRQ 21 in NVIC
  TIMER1_CTL_R = 0x00000001;    // 10) enable TIMER1A
}

void Timer1A_Handler(void){
  TIMER1_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER1A timeout
  (*PeriodicTask1)();                // execute user task
}

void Timer2_Init(void(*task)(void), unsigned long period){
  SYSCTL_RCGCTIMER_R |= 0x04;   // 0) activate timer2
  PeriodicTask2 = task;          // user function
  TIMER2_CTL_R = 0x00000000;    // 1) disable timer2A during setup
  TIMER2_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER2_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER2_TAILR_R = period-1;    // 4) reload value
  TIMER2_TAPR_R = 0;            // 5) bus clock resolution
  TIMER2_ICR_R = 0x00000001;    // 6) clear timer2A timeout flag
  TIMER2_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI5_R = (NVIC_PRI5_R&0x00FFFFFF)|0x80000000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 39, interrupt number 23
  NVIC_EN0_R = 1<<23;           // 9) enable IRQ 23 in NVIC
  TIMER2_CTL_R = 0x00000001;    // 10) enable timer2A
}

void Timer2A_Handler(void){
  TIMER2_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER2A timeout
  (*PeriodicTask2)();                // execute user task
}


void Timer3_Init(void(*task)(void), unsigned long period){
  SYSCTL_RCGCTIMER_R |= 0x08;   // 0) activate TIMER3
  PeriodicTask3 = task;          // user function
  TIMER3_CTL_R = 0x00000000;    // 1) disable TIMER3A during setup
  TIMER3_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER3_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER3_TAILR_R = period-1;    // 4) reload value
  TIMER3_TAPR_R = 0;            // 5) bus clock resolution
  TIMER3_ICR_R = 0x00000001;    // 6) clear TIMER3A timeout flag
  TIMER3_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI8_R = (NVIC_PRI8_R&0x00FFFFFF)|0x80000000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 51, interrupt number 35
  NVIC_EN1_R = 1<<(35-32);      // 9) enable IRQ 35 in NVIC
  TIMER3_CTL_R = 0x00000001;    // 10) enable TIMER3A
}

void Timer3A_Handler(void){
  TIMER3_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER3A timeout
  (*PeriodicTask3)();                // execute user task
}

void EdgeTriggeredPortA_Init(void)	{
		// Interrupts
	GPIO_PORTA_IS_R &= ~0x10;     // (d) PA4 is edge-sensitive
  GPIO_PORTA_IBE_R &= ~0x10;    //     PA4 is not both edges
  GPIO_PORTA_IEV_R |= 0x10;    //     PA4 rising edge event
  GPIO_PORTA_ICR_R = 0x10;      // (e) clear flag3
  GPIO_PORTA_IM_R |= 0x10;      // (f) arm interrupt on PA4
	NVIC_PRI0_R = (NVIC_PRI0_R&0xFFFFFF0F)|0x000000A0; // (g) priority 5
  NVIC_EN0_R |= 0x00000001;		      // (h) enable interrupt 4 in NVIC

}

void EdgeTriggeredPortB_Init(void)	{
		// Interrupts
	GPIO_PORTB_IS_R &= ~0x10;     // (d) PB4 is edge-sensitive
  GPIO_PORTB_IBE_R &= ~0x10;    //     PB4 is not both edges
  GPIO_PORTB_IEV_R |= 0x10;    //     PB4 rising edge event
  GPIO_PORTB_ICR_R = 0x10;      // (e) clear flag3
  GPIO_PORTB_IM_R |= 0x10;      // (f) arm interrupt on PB4
	NVIC_PRI0_R = (NVIC_PRI0_R&0xFFFF0FFF)|0x0000C000; // (g) priority 6
  NVIC_EN0_R |= 0x00000002;		      // (h) enable interrupt 4 in NVIC

}



void EdgeTriggeredPortC_Init(void)	{
		// Interrupts
	GPIO_PORTC_IS_R &= ~0x10;     // (d) PC4 is edge-sensitive
  GPIO_PORTC_IBE_R &= ~0x10;    //     PC4 is not both edges
  GPIO_PORTC_IEV_R |= 0x10;    //     PC4 rising edge event
  GPIO_PORTC_ICR_R = 0x10;      // (e) clear flag3
  GPIO_PORTC_IM_R |= 0x10;      // (f) arm interrupt on PC4
	NVIC_PRI0_R = (NVIC_PRI0_R&0xFF0FFFFF)|0x00200000; // (g) priority 0
  NVIC_EN0_R |= 0x00000004;		      // (h) enable interrupt 4 in NVIC

}

void EdgeTriggeredPortE_Init(void)	{
		// Interrupts
	GPIO_PORTE_IS_R &= ~0x8;     // (d) PE3 is edge-sensitive
  GPIO_PORTE_IBE_R &= ~0x8;    //     PE3 is not both edges
  GPIO_PORTE_IEV_R |= 0x8;    //     PE3 rising edge event
  GPIO_PORTE_ICR_R = 0x8;      // (e) clear flag3
  GPIO_PORTE_IM_R |= 0x8;      // (f) arm interrupt on PE3
	NVIC_PRI1_R = (NVIC_PRI1_R&0xFFFFFF0F)|0x00000000; // (g) priority 0
  NVIC_EN0_R |= 0x00000010;		      // (h) enable interrupt 4 in NVIC

}



void EdgeTriggeredPortF_Init(void){       
int delay=0;
  SYSCTL_RCGCGPIO_R |= 0x00000020; // (a) activate clock for port F
	delay++;	                       // (b) wait for clock
	delay++;
	delay++;
	delay++;
	delay++;
	delay++;
  GPIO_PORTF_DIR_R &= ~0x10;    // (c) make PF4 in (built-in button)
  GPIO_PORTF_AFSEL_R &= ~0x10;  //     disable alt funct on PF4
  GPIO_PORTF_DEN_R |= 0x10;     //     enable digital I/O on PF4
  GPIO_PORTF_PCTL_R &= ~0x000F0000; //  configure PF4 as GPIO
  GPIO_PORTF_AMSEL_R &= ~0x10;  //    disable analog functionality on PF4
  GPIO_PORTF_PUR_R |= 0x10;     //     enable weak pull-up on PF4
  GPIO_PORTF_IS_R &= ~0x10;     // (d) PF4 is edge-sensitive
  GPIO_PORTF_IBE_R &= ~0x10;    //     PF4 is not both edges
  GPIO_PORTF_IEV_R &= ~0x10;    //     PF4 falling edge event
  GPIO_PORTF_ICR_R = 0x10;      // (e) clear flag4
  GPIO_PORTF_IM_R |= 0x10;      // (f) arm interrupt on PF4
  NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|0x00A00000; // (g) priority 5
  NVIC_EN0_R |= 0x40000000;      // (h) enable interrupt 30 in NVIC
//  EnableInterrupts();           // (i) Enable global Interrupt flag (I)

}

