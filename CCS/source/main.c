//********************************************************************
//                  Yanai & Roy DCS Final project
//********************************************************************

#include  "../header/api.h"         // private library - API layer
#include  "../header/app.h"         // private library - APP layer

// FSM declare
enum FSMstate state;
enum SYSmode lpm_mode;
enum Steppermode stepper_status;
enum scriptmode script_state;

void main(void){

  state = state0;  // start in idle state on RESET
  lpm_mode = mode0;     // start in idle state on RESET
  sysConfig();  // run once to configure all CPU modules
  lcd_clear();


  while(1){
    switch(state){
      case state0: //idle
          IE2 |= UCA0RXIE; // Enable USCI_A0 RX interrupt
          enterLPM(mode0); // enter sleep mode to reduce power consumption
          break;

      case state1:
          stepper_control();  // state1 operation function: using stepper motor manually using Joystick
          break;

      case state2:
          Leonardo_de_joystick();  // state2 operation function: using the Joystick to make a painter application PC side
          break;

      case state3:
          StepperCalibration(steps_count = 0);  // state3 operation function: calibrate the motor steps for full circle and calculate
                             //the angle size - save both in flash
          break;

      case state4:
           script_mode();   // state4 operation function: upload up to 3 script from PC side (10 line max) and run
                            // their content of the CPU
        break;
    }
  }
}

