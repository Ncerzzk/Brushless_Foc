#ifndef __FOC__H
#define __FOC__H
#include "stdint.h"
void Foc_Init(float rps);
static void SVPWM(float Target_U,float duty);
static void Test_Direction();
void VF_Openloop_Control(float duty,float freq,int8_t direction,float rps);
extern uint16_t ADC_Values_Raw[];
extern uint16_t ADC_Values_Raw2[];
extern float Ialpha,Ibeta;
extern float Position_Degree;
void Theta_Handler();

void Set_to_U4(uint8_t t);
void Foc();
#endif