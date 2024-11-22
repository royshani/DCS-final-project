#ifndef HALGPIO_H_
#define HALGPIO_H_
#include "../header/bsp.h"
#include "../header/app.h"
#include "stdint.h"
extern enum FSMstate state;   // global variable
extern enum SYSmode lpm_mode; // global variable
extern enum Steppermode stepper_status;
extern enum scriptmode script_state;
extern void sysConfig(void);
extern void delay(unsigned int);
extern void DelayMs(unsigned int);
extern void DelayUs(unsigned int);
extern void enterLPM(unsigned char);
extern void enable_interrupts();
extern int divideBy360(long num);
extern void TIMERA0_ON(unsigned int clk_counter);
extern void convert_integer_to_string(char *str, unsigned int num);
extern void timerA0Onscript();
extern void send_to_pc();

extern uint32_t convert_hex_to_int(char *hex);
extern int division(int dividend, int divisor);
extern int16_t degree(int16_t y_fp, int16_t x_fp);
extern int joystick_degree();
extern char step_count[];
extern int calibration_counter;
extern int calibflag;
extern int calib_start;
extern int stepper_delay;
extern int steps_count;
extern int stepper_position;
extern int phi;
extern char file_content[];
extern char stringFromPC[];
extern char array_for_LCD[];
extern int wait_to_execute;
extern unsigned int d;
extern unsigned int ms;
extern int j;
extern char RxData[];
extern int Vx;
extern int Vy;
extern int erased_flash;
extern unsigned int joystick_location[];
extern int joydegree;
extern int Tx_index;
extern char results_to_send[];
extern int painter_state;
extern int steps_count_flash;
extern int degree_steps;
extern int steps_needed;
extern int joystick_steps;
extern int begin_manual_flag;
extern int send_xORy;
extern int send_coor;
extern char flash_buffer[];
extern char *script_pointer1, *script_pointer2, *script_pointer3;
extern char script_command[], argument1[], argument2[];  // Buffers to hold instruction opcode and operands
extern void Leonardo_de_joystick();
extern void JoyStick_Sample();
extern int file_index;
extern int send_position_flag;
extern int script_case;
extern int calibrated
;

extern __interrupt void Timer_A0(void);
extern __interrupt void Timer_A1(void);
extern __interrupt void Timer1_A0_ISR(void);
extern __interrupt void USCI0RX_ISR(void);
extern __interrupt void USCI0TX_ISR(void);
extern __interrupt void Joystick_handler(void);
#endif
/*----------------------------------------------------------------
 * LCD setup labels
 *----------------------------------------------------------------*/

#ifdef CHECKBUSY
    #define LCD_WAIT lcd_check_busy()
#else
    #define LCD_WAIT DelayMs(5)
#endif

/*----------------------------------------------------------
  CONFIG: change values according to your port pin selection
------------------------------------------------------------*/
#define LCD_EN(a)   (!a ? (P1OUT&=~0X01) : (P1OUT|=0X01)) // Set P1.0 as LCD enable pin
#define LCD_EN_DIR(a)   (!a ? (P1DIR&=~0X01) : (P1DIR|=0X01)) // Set P1.0 pin direction

#define LCD_RS(a)   (!a ? (P1OUT&=~0X40) : (P1OUT|=0X40)) // Set P1.6 as LCD RS pin
#define LCD_RS_DIR(a)   (!a ? (P1DIR&=~0X40) : (P1DIR|=0X40)) // Set P1.6 pin direction

#define LCD_RW(a)   (!a ? (P1OUT&=~0X80) : (P1OUT|=0X80)) // Set P1.7 as LCD RW pin
#define LCD_RW_DIR(a)   (!a ? (P1DIR&=~0X80) : (P1DIR|=0X80)) // Set P1.7 pin direction

#define LCD_DATA_OFFSET 0x04 //data pin selection offset for 4 bit mode, variable range is 0-4, default 0 - Px.0-3, no offset


/*---------------------------------------------------------
  END CONFIG
-----------------------------------------------------------*/
#define FOURBIT_MODE    0x0
#define EIGHTBIT_MODE   0x1
#define LCD_MODE        FOURBIT_MODE

#define OUTPUT_PIN      1
#define INPUT_PIN       0
#define OUTPUT_DATA     (LCD_MODE ? 0xFF : (0x0F << LCD_DATA_OFFSET))
#define INPUT_DATA      0x00

#define LCD_STROBE_READ(value)  LCD_EN(1), \
                asm("nop"), asm("nop"), \
                value=LCD_DATA_READ, \
                LCD_EN(0)

#define lcd_cursor(x)       lcd_cmd(((x)&0x7F)|0x80)
#define lcd_clear()         lcd_cmd(0x01)
#define lcd_putchar(x)      lcd_data(x)
#define lcd_goto(x)         lcd_cmd(0x80+(x))
#define lcd_cursor_right()  lcd_cmd(0x14)
#define lcd_cursor_left()   lcd_cmd(0x10)
#define lcd_display_shift() lcd_cmd(0x1C)
#define lcd_home()          lcd_cmd(0x02)
#define cursor_off          lcd_cmd(0x0C)
#define cursor_on           lcd_cmd(0x0F)
#define lcd_function_set    lcd_cmd(0x3C) // 8bit,two lines,5x10 dots
#define lcd_new_line        lcd_cmd(0xC0)

extern void lcd_cmd(unsigned char);
extern void lcd_data(unsigned char);
extern void lcd_puts(const char * s);
extern void lcd_init();
extern void lcd_strobe();






