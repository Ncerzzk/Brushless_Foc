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

enum _board_mode
{
    STOP_MODE,
    WAIT_MODE,
    SENSOR_FOC,
    SENSOR_LESS_FOC,
    VF_OPENLOOP,
    MEASURE_R,
    MEASURE_L,
    TEST_DIRECTION = 0xF1,
    TEST_POSITION_OFFSET,
    SING_MODE, // 唱歌模式
    VOICE_MODE // 说话模式
};

extern enum _board_mode Board_Mode;

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
void voice_3khz_handler();
void Set_Vector(Uvect_Mos u, float duty);
#endif