#include "foc.h"
#include "tim.h"
#include "math.h"
#include "arm_math.h"
#include "adc.h"


#define PAIRS   7   //Pole of Pairs
#define PART_NUM   3072 // 多少细分 
#define MIN(a,b)    a<b?a:b

uint16_t ADC_Values_Raw[4];
uint16_t ADC_Values_Raw2[4];
//#define PI  3.1415926f
typedef struct{
    uint8_t a;
    uint8_t b;
    uint8_t c;
    float theta;
    float x;
    float y;
}Uvect_Mos;

typedef struct{
    float ccra;
    float ccrb;
    float ccrc;
}CCR_Duty;

Uvect_Mos U0={0,0,0,0};
Uvect_Mos U4={1,0,0,0};
Uvect_Mos U6={1,1,0,PI/3};
Uvect_Mos U2={0,1,0,PI/3*2};
Uvect_Mos U3={0,1,1,PI};
Uvect_Mos U1={0,0,1,PI/3*4};
Uvect_Mos U5={1,0,1,PI/3*5};
Uvect_Mos U7={1,1,1,0};

Uvect_Mos *Uvects[6]={&U1,&U2,&U3,&U4,&U5,&U6};

void UVects_Init(){
    for(int i=0;i<6;++i){
        float v_theta=Uvects[i]->theta;
        Uvects[i]->x=cosf(v_theta);
        Uvects[i]->y=sinf(v_theta);
    }
}

void Foc_Init(float rps){
    // rps unit is r/s
    int interupt_time=0;
    int PSC=0;

    //interupt_time=rps*PAIRS*PART_NUM;
    //PSC=1281/interupt_time;    //84000000/65535/interrupt_time
    //TIM7->ARR=84000000/(PSC+1)/interupt_time;
    //TIM7->PSC=PSC;
    interupt_time=rps*PAIRS*PART_NUM;
    PSC=1281/interupt_time;
    TIM8->ARR=168000000/(PSC+1)/interupt_time/2;
    TIM8->PSC=PSC;
    //TIM8->ARR=6200;
    //TIM8->PSC=0;
    

    UVects_Init();
    HAL_TIM_PWM_Start(&htim8,TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim8,TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim8,TIM_CHANNEL_3);

    HAL_TIMEx_PWMN_Start(&htim8,TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim8,TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim8,TIM_CHANNEL_3);
    //HAL_TIM_Base_Start_IT(&htim7);

    //ADC
    HAL_ADC_Start_DMA(&hadc1,(uint32_t *)ADC_Values_Raw,2);
    HAL_ADC_Start_DMA(&hadc2,(uint32_t *)ADC_Values_Raw2,3);
    HAL_TIM_Base_Start_IT(&htim8);
}

uint8_t get_area(uint32_t now_theta){
    return now_theta/60+1;
}

void get_area_u(uint8_t area,Uvect_Mos *u1,Uvect_Mos *u2){
    switch (area)
    {
    case 1:
        *u1=U4;
        *u2=U6;
        break;
    case 2:
        *u1=U6;
        *u2=U2;
        break;
    case 3:
        *u1=U2;
        *u2=U3;
        break;
    case 4:
        *u1=U3;
        *u2=U1;
        break;
    case 5:
        *u1=U1;
        *u2=U5;
        break;
    case 6:
        *u1=U5;
        *u2=U4;
        break;
    default:
        break;
    }
}

void get_t(float theta,float * t1,float *t2){
/*
/       u2y x               u2x y       \
| ----------------- - ----------------- |
| u1x u2y - u2x u1y   u1x u2y - u2x u1y |
|                                       |
|       u1x y               u1y x       |
| ----------------- - ----------------- |
\ u1x u2y - u2x u1y   u1x u2y - u2x u1y /
*/
    float temp_t1,temp_t2;
    float theta_f;  // 单位为rad
    Uvect_Mos u1,u2;
    float x,y;
    get_area_u(get_area(theta),&u1,&u2);
    //theta_f=(float)theta*2.0f*PI/360;
    arm_sin_cos_f32(theta,&y,&x);
    x*=0.86f;
    y*=0.86f;
    //x=0.86f*cosf(theta_f);
    //y=0.86f*sinf(theta_f);
    temp_t1=u2.y*x/(u1.x*u2.y-u2.x*u1.y)-(u2.x*y)/(u1.x*u2.y-u2.x*u1.y);
    temp_t2=u1.x*y/(u1.x*u2.y-u2.x*u1.y)-(u1.y*x)/(u1.x*u2.y-u2.x*u1.y);

    *t1=temp_t1;
    *t2=temp_t2;

}

CCR_Duty get_ccr_duty(float t1,float t2,Uvect_Mos u1,Uvect_Mos u2){
    float t0,t3;
    CCR_Duty result;
    t0=(1-t1-t2)/2;
    t3=t0;

    result.ccra=U7.a*t3+u1.a*t1+u2.a*t2;
    result.ccrb=U7.b*t3+u1.b*t1+u2.b*t2;
    result.ccrc=U7.c*t3+u1.c*t1+u2.c*t2;

    return result;
}

// 一次theta增量为360/PART_NUM
#define delta_theta 360.0f/PART_NUM
static float theta=0;
void svpwm(){
    CCR_Duty ccr_duty;
    Uvect_Mos u1,u2;
    
    float t1,t2;
    //HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
    uint8_t area=get_area((int)theta); 
    get_area_u(area,&u1,&u2);
    get_t(theta,&t1,&t2);
    ccr_duty=get_ccr_duty(t1,t2,u1,u2);

    TIM8->CCR1=(ccr_duty.ccrc)*TIM8->ARR;
    TIM8->CCR2=(ccr_duty.ccrb)*TIM8->ARR;
    TIM8->CCR3=(ccr_duty.ccra)*TIM8->ARR;
    ///uint32_t temp=MIN(TIM8->CCR1,TIM8->CCR2);
    //temp=MIN(temp,TIM8->CCR3);   // for trig adc!
    TIM8->CCR4=1;

    theta+=delta_theta;
    if(theta>=360){
        theta-=360;
    }
    //HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
}



void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){

    if(htim->Instance==TIM7){
        //svpwm();
    }
    if(htim->Instance==TIM8){
            svpwm();
    }
}