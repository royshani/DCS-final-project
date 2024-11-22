#ifndef _app_H_
#define _app_H_


enum FSMstate{state0,state1,state2,state3,state4}; // global variable
enum SYSmode{mode0,mode1,mode2,mode3,mode4}; // global variable
enum Steppermode{rotationMode, stopMode, manualMode}; // define the stepper motor state machine
enum scriptmode{scriptupload, executescript, nameupdate, scriptreceive}; // define states for script mode in state 4

#endif





