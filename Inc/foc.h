#ifndef __FOC__H
#define __FOC__H
#include "stdint.h"
typedef struct
{
    uint8_t a;
    uint8_t b;
    uint8_t c;
    float theta;
    float x;
    float y;
} Uvect_Mos;

typedef struct
{
    float ccra;
    float ccrb;
    float ccrc;
} CCR_Duty;


void Foc_Init(float rps);

void VF_Openloop_Control(float duty,float freq,int8_t direction,float rps);
extern uint16_t ADC_Values_Raw[];
extern uint16_t ADC_Values_Raw2[];
extern float Ialpha,Ibeta;
extern float Position_Degree;
void Theta_Handler();
void Music_Play_Beat();

void Set_to_U4(uint8_t t);
void Foc();
#endif