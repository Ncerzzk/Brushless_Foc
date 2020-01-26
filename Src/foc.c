#include "foc.h"
#include "tim.h"
#include "math.h"
#include "arm_math.h"
#include "adc.h"
#include "as5047.h"
#include "uart_ext.h"

#define MIN(a,b)    a<b?a:b

uint16_t ADC_Values_Raw[4];
uint16_t ADC_Values_Raw2[4];
float Ialpha,Ibeta;
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

typedef struct{
    uint8_t pairs;
    float R;
    float L;
    int8_t direction; 
    // 电机的运转方向，约定在开环模式下，电机按u4->u6方向运行（电角度增大），机械角度也增大为正方向
    // 此参数与电机 a b c 三相的焊接有关
    // 此参数仅在有感模式中(SENSOR_FOC)有用
}Motor_Info;

enum{
    WAIT,
    SENSOR_FOC,
    SENSOR_LESS_FOC,
    VF_OPENLOOP,
    MEASURE_R,
    MEASURE_L,
    TEST_DIRECTION,
    TEST_POSITION_OFFSET
}Board_Mode;

struct{
    uint32_t time; // unit:ms
    float last_position;
    int direction_result;
}Test_Direction_Info;

Motor_Info Motor={7,0,0};

Uvect_Mos U0={0,0,0,0};
Uvect_Mos U4={1,0,0,0};
Uvect_Mos U6={1,1,0,PI/3};
Uvect_Mos U2={0,1,0,PI/3*2};
Uvect_Mos U3={0,1,1,PI};
Uvect_Mos U1={0,0,1,PI/3*4};
Uvect_Mos U5={1,0,1,PI/3*5};
Uvect_Mos U7={1,1,1,0};

Uvect_Mos *Uvects[6]={&U1,&U2,&U3,&U4,&U5,&U6};


static float theta=0;
float RPS=6;   // 转速

float Position_Degree;

float Position_Offset=215.793457f;
//float Position_Offset=273.164062f;

#define TIM7_FREQ   1000
#define TIM8_FREQ   20000

#define DRIVE

static void UVects_Init(){
    for(int i=0;i<6;++i){
        float v_theta=Uvects[i]->theta;
        Uvects[i]->x=cosf(v_theta);
        Uvects[i]->y=sinf(v_theta);
    }
}

void Foc_Init(float rps){
    // rps unit is r/s
    Board_Mode=VF_OPENLOOP;
    TIM8->ARR=168000000/TIM8_FREQ/2;

    TIM8->CCR4=TIM8->ARR-2;

    UVects_Init();
#ifdef DRIVE 
    HAL_TIM_PWM_Start(&htim8,TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim8,TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim8,TIM_CHANNEL_3);

    HAL_TIMEx_PWMN_Start(&htim8,TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim8,TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim8,TIM_CHANNEL_3);
#endif
    TIM7->ARR=84000000/TIM7_FREQ-1;
    HAL_TIM_Base_Start_IT(&htim7);

    //ADC
   // HAL_ADC_Start_DMA(&hadc1,(uint32_t *)ADC_Values_Raw,2);
    HAL_ADC_Start_DMA(&hadc2,(uint32_t *)ADC_Values_Raw2,2);
    HAL_TIM_Base_Start(&htim8);
}

uint8_t get_area(uint32_t now_theta){
    return now_theta/60+1;
}

// 根据扇区号，获取该扇区的两个电压矢量
// area:1-6
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


/*
    根据theta来计算相邻两个矢量的作用时间的函数，
    因为用到除法次数多了一点，目前暂时废弃。
*/
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
    y*=0.86f;   // sqrt(3)/2 没错
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

void SVPWM_Step(CCR_Duty duty){
    TIM8->CCR1=duty.ccrc*TIM8->ARR;
    TIM8->CCR2=duty.ccrb*TIM8->ARR;
    TIM8->CCR3=duty.ccra*TIM8->ARR;
}

/*
    将电压矢量设置为U4，并持续一段时间
*/
void Set_to_U4(uint8_t t){
    CCR_Duty temp_duty={0};
    temp_duty.ccra=U4.a;
    temp_duty.ccrb=U4.b;
    temp_duty.ccrc=U4.c;
    SVPWM_Step(temp_duty);
    HAL_Delay(t);
    temp_duty.ccra=U0.a;
    temp_duty.ccrb=U0.b;
    temp_duty.ccrc=U0.c;
    SVPWM_Step(temp_duty);    
    uprintf("position:%f \r\n",Position_Degree);
}


extern uint8_t Init_OK;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){

    if(htim->Instance==TIM7){
        if(!Init_OK){
            return ;
        }
        Theta_Handler();
    }
}


void SVPWM_Normal_CalT1T2(float alpha,float beta,uint8_t area,float *t1,float *t2){
    float temp=0;
    switch(area){
        case 1:
            *t1=alpha - 0.57735f*beta;
            *t2=1.1547f*beta;
        break;
        case 2:
            *t1=alpha + 0.57735f*beta;
            *t2=0.57735f*beta - alpha;
            break;
        case 3:
            *t1=1.1547f*beta;
            *t2=-alpha - 0.57735f*beta;
            break;
        case 4:
            *t1=0.57735f*beta - alpha;
            *t2=- 1.1547f*beta;
            break;
        case 5:
            temp=- 0.57735f*beta;
            *t1=temp-alpha;
            *t2=temp+alpha;
            break;
        case 6:
            *t1=-1.1547f*beta;
            *t2=alpha + 0.57735f*beta;
            break;
    }
}


//#define dealtK PAIRS*360.0f/TIM7_FREQ
#define dealtK 360.0f/TIM7_FREQ
void Theta_Handler(){
    Uvect_Mos u1,u2;
    float t1,t2;
    float x,y;
    float dealt=RPS*dealtK;

    Position_Degree=Get_Position_Rad_Dgree(1);
    switch(Board_Mode){
        case SENSOR_FOC:
            Foc();
            break;
        case VF_OPENLOOP:
            VF_Openloop_Control(1,1000,1,1);
            break;
        case TEST_DIRECTION:
            Test_Direction();
            break;
        case WAIT:
            SVPWM(0,0);
            break;
        default:
        break;

    }
}

#define ADC_RAW2_CURRENT(raw) raw*3300.0f/4096/200/0.003f

void Park_Conv(float ia,float ib,float * ialpha,float *ibeta){
    *ialpha=ia;
    *ibeta=(sqrtf(3)*ib+sqrtf(3)*(ib+ia))/3.0f;
}

 void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
     float ia,ib;

     if(hadc->Instance==ADC1){

     }else if(hadc->Instance==ADC2){
        ia=ADC_RAW2_CURRENT(ADC_Values_Raw2[0]);
        ib=ADC_RAW2_CURRENT(ADC_Values_Raw2[1]);
        Park_Conv(ia,ib,&Ialpha,&Ibeta);
     }
 }

 void Foc(){
    Uvect_Mos u1,u2;
    float t1,t2;
    float x,y;

    float electric_position=(-1*Position_Degree+360);
    float electric_position_offset=(-1*Position_Offset+360);
    //float electric_position=Position_Degree;
    //float electric_position_offset=Position_Offset;
     int32_t sub=(int32_t)(electric_position-electric_position_offset);
     
    if(sub<0){
        sub+=360;
    }
    sub*=Motor.pairs;
    sub+=90;

    sub%=360;

    SVPWM(sub,1);
 }

// Target_U 期望电压矢量的角度 单位° 0-360
// duty 期望占空比 0-1
static void SVPWM(float Target_U,float duty){
    Uvect_Mos u1,u2;
    float x,y;
    float t1,t2;
    static CCR_Duty CCR_Dutys={0};

    uint8_t area=get_area(Target_U);
    get_area_u(area,&u1,&u2);
    arm_sin_cos_f32(Target_U,&y,&x);
    x*=0.86f*duty;
    y*=0.86f*duty;   // sqrt(3)/2 没错
    SVPWM_Normal_CalT1T2(x,y,area,&t1,&t2);
    CCR_Dutys=get_ccr_duty(t1,t2,u1,u2);
    SVPWM_Step(CCR_Dutys);    
}


void VF_Openloop_Control(float duty,float freq,int8_t direction,float rps){
    static float theta=0;
    float delta_p=rps*Motor.pairs*360.0f/freq;   // 每次调用本函数要增加的值

    theta+=direction*delta_p;
    if(theta>360){
        theta-=360;
    }else if(theta<0){
        theta+=360;
    }
    SVPWM(theta,duty);
}


static void Test_Direction(){
   Test_Direction_Info.time+=1;

   if(Position_Degree>Test_Direction_Info.last_position){
       Test_Direction_Info.direction_result+=1;
   }else{
       Test_Direction_Info.direction_result-=1;
   }

   VF_Openloop_Control(0.3f,1000,1,10);


   if(Test_Direction_Info.time>10000){ // run 10s
    Board_Mode=WAIT;
    Test_Direction_Info.time=0;
    uprintf_polling("direction result:%d\r\n",Test_Direction_Info.direction_result);
    if(Test_Direction_Info.direction_result>0){
        uprintf_polling("the direction of the motor is postive!\r\n");
        Motor.direction=1;
    }else{
        uprintf_polling("the direction of the motor is nagative!\r\n");
        Motor.direction=-1;
    }
    Test_Direction_Info.direction_result=0;
   }
}