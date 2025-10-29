// Lab3_Timersmain.c

/* This example accompanies the books
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
       ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2017
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
       ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2017
   "Embedded Systems: Real-Time Operating Systems for ARM Cortex-M Microcontrollers",
       ISBN: 978-1466468863, , Jonathan Valvano, copyright (c) 2017
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/

Simplified BSD License (FreeBSD License)
Copyright (c) 2017, Jonathan Valvano, All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are
those of the authors and should not be interpreted as representing official
policies, either expressed or implied, of the FreeBSD Project.
*/
// Negative logic bump sensors
// P8.7 Bump5
// P8.6 Bump4
// P8.5 Bump3
// P8.4 Bump2
// P8.3 Bump1
// P8.0 Bump0

// Sever VCCMD=VREG jumper on Motor Driver and Power Distribution Board and connect VCCMD to 3.3V.
//   This makes P3.7 and P3.6 low power disables for motor drivers.  0 to sleep/stop.
// Sever nSLPL=nSLPR jumper.
//   This separates P3.7 and P3.6 allowing for independent control
// Left motor direction connected to P1.7 (J2.14)
// Left motor PWM connected to P2.7/TA0CCP4 (J4.40)
// Left motor enable connected to P3.7 (J4.31)
// Right motor direction connected to P1.6 (J2.15)
// Right motor PWM connected to P2.6/TA0CCP3 (J4.39)
// Right motor enable connected to P3.6 (J2.11)

#include "msp.h"
#include "..\inc\Clock.h"
#include "..\inc\SysTick.h"
#include "..\inc\CortexM.h"
#include "..\inc\LaunchPad.h"
#include "..\inc\Motor.h"
#include "..\inc\TimerA1.h"
#include "..\inc\TExaS.h"
#include "..\inc\Reflectance.h"
// new
#include "..\inc\Tachometer.h"
#include "..\inc\TA3InputCapture.h"
#include "..\inc\PWM.h"
#include "..\inc\Bump.h"
// new: for IR
#include "..\inc\ADC14.h"
#include "..\inc\IRDistance.h"
#include "../inc/LPF.h"
#include "../inc/UART0.h"
#include "../inc/BaseConvert.h"

volatile uint8_t bumpState;
volatile uint8_t status;

volatile uint8_t reflectance_data, bump_data;

volatile uint8_t collision_detected = 0;
volatile uint8_t collision_bumpstate = 0;

// Driver test
void TimedPause(uint32_t time){
  Clock_Delay1ms(time);          // run for a while and stop
  Motor_Stop();
  while(LaunchPad_Input()==0);  // wait for touch
  while(LaunchPad_Input());     // wait for release
}

// Test of Periodic interrupt
#define REDLED (*((volatile uint8_t *)(0x42098060)))
#define BLUELED (*((volatile uint8_t *)(0x42098068)))
uint32_t Time;

#define SPEED 1000

//void Task(uint8_t bumpstate){
//    if(bumpstate != 0x3F){
//        Motor_Stop();              // Only stop
//        collision_detected = 1;     // Set flag
//        collision_bumpstate = bumpstate;  // Save which bumps
//    }
//}

volatile uint32_t ADCvalue;
volatile uint32_t ADCflag;
volatile uint32_t nr,nc,nl;
//#define IR_THRESHOLD 50
volatile uint32_t left, center, right;

void SensorRead_ISR(void){  // runs at 2000 Hz
  uint32_t raw17,raw12,raw16;
  P1OUT ^= 0x01;         // profile
  P1OUT ^= 0x01;         // profile
  ADC_In17_12_16(&raw17,&raw12,&raw16);  // sample
  nr = LPF_Calc(raw17);  // right is channel 17 P9.0
  nc = LPF_Calc2(raw12);  // center is channel 12, P4.1
  nl = LPF_Calc3(raw16);  // left is channel 16, P9.1
  ADCflag = 1;           // semaphore
  P1OUT ^= 0x01;         // profile
}

int main(void){
    // Uses Timer generated PWM to move the robot
    // Uses TimerA1 to periodically
    // check the bump switches, stopping the robot on a collision


    uint32_t raw12, raw16, raw17;
    int32_t n; uint32_t s;
    s = 256; // replace with your choice

    Clock_Init48MHz();  //SMCLK=12Mhz
    ADCflag = 0;
    s = 256; // replace with your choice
    ADC0_InitSWTriggerCh17_12_16();   // initialize channels 17,12,16
    ADC_In17_12_16(&raw17,&raw12,&raw16);  // sample
    LPF_Init(raw17,s);     // P9.0/channel 17
    LPF_Init2(raw12,s);     // P4.1/channel 12
    LPF_Init3(raw16,s);     // P9.1/channel 16
    UART0_Init();          // initialize UART0 115,200 baud rate
    LaunchPad_Init();
    TimerA1_Init(&SensorRead_ISR,250);    // 2000 Hz sampling

    Bump_Init();      // bump switches
    Motor_Init();     // your function
    Tachometer_Init();
    TExaS_Init(LOGICANALYZER_P2);
    bumpState = 0x3F;           // FB: to prevent the motor to stop immediately, because the initial value of bumpState is 0x00 otherwise.
//    TimerA1_Init(&Task,50000);  // 10 Hz
    EnableInterrupts();

    // IR setup


    TimedPause(1000);
    while(1){
        for(n=0; n<2000; n++){
          while(ADCflag == 0){};
          ADCflag = 0; // show every 2000th point
        }
        // Check for collision first
        if (bumpState != 0x3F) {


            // Handle collision here
            Motor_ForwardDist(-10, SPEED, SPEED);

            if ((~bumpState & 0b000001) || (~bumpState & 0b000010)) {
                Motor_RotateAngle(-90, SPEED);  // Right bump, turn left
            }
            else if ((~bumpState & 0b010000) || (~bumpState & 0b100000)) {
                Motor_RotateAngle(90, SPEED);   // Left bump, turn right
        }
            else {
                Motor_RotateAngle(-90, SPEED);  // Middle bump, turn left
            }
            bumpState = 0x3F; // reset bumpState
            continue;  // Skip rest of loop, start fresh
        }
        // IR measurements
        // read raw ADC values from all three sensors
        ADC_In17_12_16(&raw17,&raw12,&raw16);  // sample

        // convert raw ADC to distance in millimeters
        left = LeftConvert(nl);
        center = CenterConvert(nc);
        right = RightConvert(nr);

        if (center < 10) {
//            Motor_Stop();
            Motor_ForwardDist(-5, SPEED, SPEED);
            // turn left
            Motor_RotateAngle(-90, SPEED);
        }
        else if (left < 7) {
//            Motor_Stop();
            Motor_ForwardDist(-5, SPEED, SPEED);
            // turn right
            Motor_RotateAngle(90, SPEED);
        }
        else if (right < 7) {
//            Motor_Stop();
            Motor_ForwardDist(-5, SPEED, SPEED);
            // turn left
            Motor_RotateAngle(-90, SPEED);
        }
        else {
            Motor_Forward(SPEED, SPEED);
            Clock_Delay1ms(100);
        }
    }
}
