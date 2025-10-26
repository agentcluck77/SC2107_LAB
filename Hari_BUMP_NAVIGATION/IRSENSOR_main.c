#include <stdint.h>
#include "msp.h"
#include "../inc/Clock.h"
#include "../inc/CortexM.h"
#include "../inc/IRDistance.h"
#include "../inc/TimerA1.h"
#include "../inc/UART0.h"
#include "../inc/LaunchPad.h"
#include "../inc/ADC14.h"
#include "../inc/LPF.h"
#include "../inc/Motor.h"
#include "../inc/Bump.h"       // <<< === NEW
#include "../inc/PWM.h"


#define OBSTACLE_DISTANCE 150       // mm
#define MOTOR_SPEED 1500
#define BACKUP_TIME 500
#define TURN_TIME   400

volatile uint32_t nr, nc, nl;
volatile uint32_t left, center, right;
volatile uint32_t ADCflag;

volatile uint8_t bumpState;

void SensorRead_ISR(void) {
    uint32_t raw17, raw12, raw16;
    P1OUT ^= 0x01;
    P1OUT ^= 0x01;
    ADC_In17_12_16(&raw17, &raw12, &raw16);
    nr = LPF_Calc(raw17);
    nc = LPF_Calc2(raw12);
    nl = LPF_Calc3(raw16);
    ADCflag = 1;
    P1OUT ^= 0x01;
}


int main(void) {
    uint32_t raw17, raw12, raw16;
    int32_t n;
    uint32_t s = 256;

    Clock_Init48MHz();
    ADCflag = 0;
    ADC0_InitSWTriggerCh17_12_16();
    ADC_In17_12_16(&raw17, &raw12, &raw16);
    LPF_Init(raw17, s);
    LPF_Init2(raw12, s);
    LPF_Init3(raw16, s);

    UART0_Init();
    LaunchPad_Init();
    Motor_Init();
    TimerA1_Init(&SensorRead_ISR, 250); // 2000 Hz
    Bump_Init();             // <<< === Init bump switches with callback

    UART0_OutString("Robot Obstacle Avoidance System Initialized\n");
    EnableInterrupts();
    bumpState = 0x3F;
    while (1) {
        // Wait for next ADC sample
        for (n = 0; n < 2000; n++) {
            while (ADCflag == 0);
            ADCflag = 0;
        }

        left = LeftConvert(nl);
        center = CenterConvert(nc);
        right = RightConvert(nr);

        // Debug: show IR distances
        UART0_OutUDec5(left); UART0_OutString(" mm, ");
        UART0_OutUDec5(center); UART0_OutString(" mm, ");
        UART0_OutUDec5(right); UART0_OutString(" mm\r\n");
        //Below code works with bup and IR, but IR is too naive
        /*
        if (bumpState!=0x3F) {
            // Stop and back up
            Motor_Stop();
            Clock_Delay1ms(100);
            Motor_Backward(MOTOR_SPEED, MOTOR_SPEED);
            Clock_Delay1ms(BACKUP_TIME);
            bumpState = 0x3F;

        }
        else if (center < OBSTACLE_DISTANCE || left < OBSTACLE_DISTANCE || right < OBSTACLE_DISTANCE) {
            Motor_Stop();
            Clock_Delay1ms(100);

            Motor_Backward(MOTOR_SPEED, MOTOR_SPEED);
            Clock_Delay1ms(BACKUP_TIME);
            Motor_Stop();
            Clock_Delay1ms(100);

            if (left < right) {
                Motor_Right(MOTOR_SPEED, MOTOR_SPEED);
            } else {
                Motor_Left(MOTOR_SPEED, MOTOR_SPEED);
            }

            Clock_Delay1ms(TURN_TIME);
            Motor_Stop();
            Clock_Delay1ms(100);
        }
        else {
            Motor_Forward(MOTOR_SPEED, MOTOR_SPEED);
        }
        */

        if (center < OBSTACLE_DISTANCE) {
            // Obstacle in front, move straight backward
            Motor_Stop();
            Clock_Delay1ms(100);

            Motor_Backward(MOTOR_SPEED, MOTOR_SPEED);
            Clock_Delay1ms(BACKUP_TIME);
            Motor_Stop();
            Clock_Delay1ms(100);
        }
        else if (left < OBSTACLE_DISTANCE) {
            // Obstacle on left, turn right by moving right wheel more
            Motor_Stop();
            Clock_Delay1ms(100);

            // Slow down left wheel, speed up right wheel to turn right
            Motor_Right(MOTOR_SPEED, MOTOR_SPEED);
            Clock_Delay1ms(TURN_TIME);
            Motor_Stop();
            Clock_Delay1ms(100);
        }
        else if (right < OBSTACLE_DISTANCE) {
            // Obstacle on right, turn left by moving left wheel more
            Motor_Stop();
            Clock_Delay1ms(100);

            // Speed up left wheel, slow down right wheel to turn left
            Motor_Left(MOTOR_SPEED, MOTOR_SPEED / 5);
            Clock_Delay1ms(TURN_TIME);
            Motor_Stop();
            Clock_Delay1ms(100);
        }
        else {
            // No obstacle detected, move forward
            Motor_Forward(MOTOR_SPEED, MOTOR_SPEED);
        }


    }
}
