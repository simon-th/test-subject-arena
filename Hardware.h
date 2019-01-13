#include <stdint.h>

void Button_Init(void);

void Timer0_Init(void(*task)(void), uint32_t period);
void Timer0A_Handler(void);
void Timer1_Init(void(*task)(void), uint32_t period);
void Timer1A_Handler(void);
void Timer2_Init(void(*task)(void), uint32_t period);
void Timer2A_Handler(void);
void Timer3_Init(void(*task)(void), uint32_t period);
void Timer3A_Handler(void);
void EdgeTriggeredPortA_Init(void);
void EdgeTriggeredPortB_Init(void);
void EdgeTriggeredPortC_Init(void);
void EdgeTriggeredPortE_Init(void);
void EdgeTriggeredPortF_Init(void);
void IO_Touch(void);
