#ifndef __FOC__H
#define __FOC__H
#include "stdint.h"
void Foc_Init(float rps);
extern uint16_t ADC_Values_Raw[];
extern uint16_t ADC_Values_Raw2[];
extern float Position_Degree;
void Theta_Handler();

void Set_to_U4(uint8_t t);
void Foc();
#endif