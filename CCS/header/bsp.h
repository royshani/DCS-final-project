#ifndef _bsp_H_
#define _bsp_H_

#include  <msp430g2553.h>          // MSP430x2xx
//#include  <msp430xG46x.h>  // MSP430x4xx

//    General labels
#define   debounceVal      800
//#define     HIGH    1
//#define     LOW     0

//    LCD labels
#define LCD_DATA_WRITE    P2OUT //pins 2.4-2.7
#define LCD_DATA_DIR      P2DIR //pins 2.4-2.7
#define LCD_DATA_SEL      P2SEL //pins 2.4-2.7
#define LCD_CTL_SEL       P1SEL //control signals pins 1.6-1.7,1.0
#define LCD_DATA_READ     P2IN  //pins 2.4-2.7

// Joystick labels
#define joy_stickOUT         P1OUT //pin 1.5
#define joy_stickSEL         P1SEL //pins 1.5
#define joy_stickDIR         P1DIR //pins 1.5
#define joy_stickIN          P1IN  //pins 1.5
#define joy_stickIntEdgeSel  P1IES //pin 1.5 // *p1 int sel*
#define joy_stickREN         P1REN //pin 1.5
#define joy_stickIntEN       P1IE  //pin 1.5
#define joy_stickIntPend     P1IFG //pin 1.5

// Stepper Motor labels
#define SteppperOUT     P2OUT  //pins 2.0-2.3
#define StepperSEL      P2SEL  //pins 2.0-2.3
#define StepperDIR      P2DIR  //pins 2.0-2.3

// UART labels
#define TXD BIT2 //pin 2.1 for transmission
#define RXD BIT1 //pin 2.2 for receiving

extern void GPIOconfig(void);
extern void ADCconfig(void);
extern void UART_init(void);
extern void TIMER_A0_config(unsigned int clk_counter);

#endif
