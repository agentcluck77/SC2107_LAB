
#include <stdint.h>
#include "msp.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/Texas.h"
#include "../inc/Reflectance.h"  //OHL
#include "../inc/Motor.h"
#include "../inc/PWM.h"
#include "../inc/Tachometer.h"
#include "../inc/TA3InputCapture.h"
#include "../inc/CortexM.h"

/*(Left,Right) Motors, call LaunchPad_Output (positive logic)
3   1,1     both motors, yellow means go straight
2   1,0     left motor,  green  means turns right
1   0,1     right motor, red    means turn left
0   0,0     both off,    dark   means stop
(Left,Right) Sensors, call LaunchPad_Input (positive logic)
3   1,1     both buttons pushed means on line,
2   1,0     SW2 pushed          means off to right
1   0,1     SW1 pushed          means off to left
0   0,0     neither button      means lost
 */

// Linked data structure
struct State {
  uint32_t out;                // 2-bit output
  uint32_t delay;              // time to delay in 1ms
  const struct State *next[4]; // Next if 2-bit input is 0-3
};
typedef const struct State State_t;

#define Center          &fsm[0]
#define Left1           &fsm[1]
#define Left2           &fsm[2]
#define Left_off1       &fsm[3]
#define Left_off2       &fsm[4]
#define Left_stop       &fsm[5]
#define Right1          &fsm[6]
#define Right2          &fsm[7]
#define Right_off1      &fsm[8]
#define Right_off2      &fsm[9]
#define Right_stop      &fsm[10]

//OHL
State_t fsm[11]={
  {0x03,  500, { Center,         Left1,      Right1,     Center    }},   // Center
  {0x02,  500, { Left_off1,      Left2,      Right1,     Center    }},   // Left1
  {0x03,  500, { Left_off1,      Left1,      Right1,     Center    }},   // Left2
  {0x02, 500, { Left_off2,      Left_off2,  Left_off2,  Left_off2 }},   // Left_off1
  {0x03, 500, { Left_stop,      Left1,      Right1,     Center    }},   // Left_off2
  {0x00,  0, { Left_stop,      Left_stop,  Left_stop,  Center }},   // Left_stop
  {0x01,  500, { Right_off1,     Left1,      Right2,     Center    }},   // Right1
  {0x03, 500, { Right_off1,     Left1,      Right1,     Center    }},   // Right2
  {0x01, 500, { Right_off2,     Right_off2, Right_off2, Right_off2}},   // Right_off1
  {0x03,  500, { Right_stop,     Left1,      Right1,     Center    }},   // Right_off2
  {0x00,  0, { Right_stop,     Right_stop, Right_stop, Right_stop}}    // Right_stop
};

// Motor speed for line following
#define MOTOR_SPEED 500
#define MOTOR_TURNING 1000
// Function to control motors based on FSM output
void ControlMotors(uint32_t output) {
    switch(output) {
        case 0x00:  // both off, stop
            Motor_Stop();
            break;
        case 0x01:  // right motor only, turn left
            Motor_Left(MOTOR_TURNING, MOTOR_TURNING);
            break;
        case 0x02:  // left motor only, turn right
            Motor_Right(MOTOR_TURNING, MOTOR_TURNING);
            break;
        case 0x03:  // both motors, go straight
            Motor_Forward(MOTOR_SPEED, MOTOR_SPEED);
            break;
        default:
            Motor_Stop();
            break;
    }
}

State_t *Spt;  // pointer to the current state
uint32_t Input;
uint32_t Output;
/*Run FSM continuously
1) Output depends on State (LaunchPad LED)
2) Wait depends on State
3) Input (LaunchPad buttons)
4) Next depends on (Input,State)
 */
int main(void){ uint32_t heart=0;
  Clock_Init48MHz();
  Reflectance_Init();
  LaunchPad_Init();
  Motor_Init();  // Initialize motor control
  TExaS_Init(LOGICANALYZER);  // Reflectance sensor output
  Spt = Center;
  // Wait for button press to start
  while(LaunchPad_Input()==0);  // wait for touch
  while(LaunchPad_Input());      // wait for release
  while(1){
    Output = Spt->out;            // set output from FSM
    ControlMotors(Output); // new
    LaunchPad_Output(Output);     // do output to two motors
    TExaS_Set(Input<<2|Output);   // optional, send data to logic analyzer
    Clock_Delay1ms(Spt->delay);   // wait
    //Input = LaunchPad_Input();    // read sensors
    Input = Reflectance_Center(1000);
    Spt = Spt->next[Input];       // next depends on input and state
    heart = heart^1;
    LaunchPad_LED(heart);         // optional, debugging heartbeat
  }
}

// Color    LED(s) Port2
// dark     ---    0
// red      R--    0x01
// blue     --B    0x04
// green    -G-    0x02
// yellow   RG-    0x03
// sky blue -GB    0x06
// white    RGB    0x07
// pink     R-B    0x05
