// Lab3_Timersmain.c

#include "msp.h"
#include "..\inc\Clock.h"
#include "..\inc\SysTick.h"
#include "..\inc\CortexM.h"
#include "..\inc\LaunchPad.h"
#include "..\inc\Motor.h"
#include "..\inc\TimerA1.h"
#include "..\inc\TExaS.h"
#include "..\inc\Reflectance.h"
#include "..\inc\Tachometer.h"
#include "..\inc\TA3InputCapture.h"
#include "..\inc\PWM.h"
#include "..\inc\ADC14.h"
#include "..\inc\IRDistance.h"
#include <stdint.h>
#define RED 0x01
#define SPEED 1000


// Check if robot has encountered a line
volatile uint8_t reflectance;
uint8_t CheckForLine(void) {
    reflectance = Reflectance_Read(1000);
    // If any of the center sensors detect a line (black)
    if (reflectance != 0x00) {
        return 1;  // Line detected
    }
    return 0;  // No line
}

// Driver test
void TimedPause(uint32_t time){
  Clock_Delay1ms(time);          // run for a while and stop
  Motor_Stop();
  while(LaunchPad_Input()==0);  // wait for touch
  while(LaunchPad_Input());     // wait for release
}


//Initialise GPIO Port2 registers, for the RED LED
void Port2_Init(void){
  P2->SEL0 = 0x00;
  P2->SEL1 = 0x00;                        // configure P2.2-P2.0 as GPIO
  P2->DS = 0x07;                          // make P2.2-P2.0 high drive strength
  P2->DIR = 0x07;                         // make P2.2-P2.0 out
  P2->OUT = 0x00;                         // all LEDs off
}


uint8_t Data; // QTR-8RC
int32_t Position; // 332 is right, and -332 is left of center
uint8_t lineCount = 0;
int main(void){
    // Initialize all subsystems
    Clock_Init48MHz();
    LaunchPad_Init();
    Motor_Init();
    Tachometer_Init();
    Reflectance_Init();  // Initialize reflectance sensors
    TExaS_Init(LOGICANALYZER_P2);
    EnableInterrupts();
  Port2_Init();
    Data = Reflectance_Read(1000);


//    if (Data == 0b11111111){
////        P2->OUT ^= RED; // turn on RED LED when line detected
//        if (lineCount == 0) {
//            Motor_RotateAngle(90, SPEED);
//        }
//
//        Clock_Delay1ms(500);
//        Data = Reflectance_Read(1000);
//    }
//    P2->OUT &= ~RED;
    TimedPause(500);
    while (CheckForLine() == 0) {
        // move forward
        Motor_ForwardDist(10,1000,1000);
        Data = Reflectance_Read(1000);

    }
    Motor_RotateAngle(90,1000);
    Motor_ForwardDist(15,1000,1000);
    Motor_RotateAngle(-90,1000);

    while (CheckForLine() == 0) {
        // move forward
        Motor_Forward(1000,1000);
        Data = Reflectance_Read(1000);

    }
    Motor_RotateAngle(-90,1000);
    while (CheckForLine() == 0) {
        // move forward
        Motor_Forward(1000,1000);
        Data = Reflectance_Read(1000);

    }
    Motor_Stop();
    while(1);

    Clock_Delay1ms(10);
}
