#ifndef _api_H_
#define _api_H_

#include  "../header/halGPIO.h"     // private library - HAL layer

extern void StepperCalibration(int steps_count);
extern void Calibrate_step();

extern void JoyStickADC_Painter();
extern void JoyStickADC_Steppermotor();
extern void Stepper_clockwise(long);
extern void Stepper_counter_clockwise(long);
extern void Stepper_clockwise_calibaration(long);
extern void blinkRGB();
extern void clear_counters();
extern void change_delay_time();
extern void calibrate();
extern void ScriptFunc();
extern void execute_file();
extern void ClearLEDsRGB();
extern void countUP();
extern void countDOWN();
extern void rra_LCD();
extern void counter_clockwise_step(int stepper_period);
extern void clockwise_step(int stepper_period);
extern void Manual_stepper();
//extern void JoyStick_Sample();
extern void go_to_degree();
extern void draw_joystick();
extern void send_calibration(int to_send1, int to_send2);
//extern void Leonardo_de_joystick();
extern void stepper_control();
//extern int16_t Vrx;
//extern int16_t Vry;
extern void script_mode();
extern void send_position_to_pc(int position);
extern void go_to_degree_exe(int degree);






#endif







