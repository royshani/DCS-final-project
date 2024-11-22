							main.c - Function Descriptions


Overview
The main.c file implements the core Finite State Machine (FSM) of the system, controlling various operational modes based on the system's state.

Functions
void main(void): Initializes the system, enters an infinite loop, and handles state transitions to execute the corresponding operation functions.

state0 - Idle State: Reduces power consumption by enabling the RX interrupt and entering low-power mode until an event occurs.

state1 - Manual Stepper Motor Control: Executes stepper_control() to allow manual control of the stepper motor using a joystick.

state2 - Joystick-Based Painter Application: Executes Leonardo_de_joystick() to control a PC-based painting application with joystick input.

state3 - Stepper Motor Calibration: Executes StepperCalibration() to calibrate the motor's steps per full rotation and calculate the step angle, saving the results to flash memory.

state4 - Script Mode: Executes script_mode() to upload and run scripts from the PC on the microcontroller.




- main.h - includes all functions and variables extern pointers for the system robastic operation.







							api.c - Function Descriptions


Overview
The api.c file contains high-level application functions for controlling the stepper motor, handling joystick input, managing calibration, and executing scripts.

Functions
void stepper_control(): Controls the stepper motor by rotating between different modes (rotation, stop, and manual) based on user input.

void Manual_stepper(): Implements manual control of the stepper motor using the joystick, calculating the necessary steps to reach the desired angle.

void go_to_degree(int degree): Moves the stepper motor to a specified degree based on calibration data, adjusting for the shortest path.

void Leonardo_de_joystick(): Samples joystick position and sends coordinates to the PC for controlling a painter application.

void JoyStick_Sample(): Samples joystick input multiple times to reduce noise, averaging the position for accurate readings.

void StepperCalibration(int steps_count): Calibrates the stepper motor by counting steps for a full rotation, calculating the step angle, and saving the results to flash memory.

void counter_clockwise_step(int stepper_period): Rotates the stepper motor counterclockwise for a specified period.

void clockwise_step(int stepper_period): Rotates the stepper motor clockwise for a specified period.

void send_calibration(int to_send1, int to_send2): Sends calibration data (steps count and step angle) to the PC.

void script_mode(): Manages script upload and execution, handling different script-related states.

void execute_file(void): Executes the uploaded script by processing each command, adjusting the system state based on the opcode.

void go_to_degree_exe(int degree): Executes the movement of the stepper motor to a specific degree as part of the script execution.

void countUP(int argument_int): Counts up from 0 to a specified value, displaying the count on the LCD.

void countDOWN(int argument_int): Counts down from a specified value to 0, displaying the count on the LCD.

void rra_LCD(int rotate_char): Rotates a character or integer across the LCD, creating a visual effect by moving the character across the screen.




- api.h - includes all functions and variables extern pointers for the system robastic operation.







						halGPIO.c - Function Descriptions


Overview
The halGPIO.c file contains hardware abstraction layer (HAL) functions, including system configuration, LCD management, delay handling, low-power mode control, and interrupt service routines (ISRs).

Functions
void sysConfig(void): Configures system modules such as GPIO, ADC, LCD, and UART for operation.

void lcd_init(): Initializes the LCD by configuring data lines, mode settings, and display parameters.

void lcd_cmd(unsigned char c): Sends a command to the LCD, adjusting for 4-bit or 8-bit mode.

void lcd_data(unsigned char c): Sends data to the LCD, handling both 4-bit and 8-bit modes.

void lcd_strobe(): Generates an enable signal to latch data or command into the LCD.

void lcd_puts(const char * s): Writes a string of characters to the LCD display.

void DelayUs(unsigned int cnt): Creates a microsecond delay using a simple loop.

void DelayMs(unsigned int cnt): Creates a millisecond delay by looping the microsecond delay function.

void delay(unsigned int t): Implements a generic delay by looping through a counter.

void enterLPM(unsigned char LPM_level): Puts the system into a specified low-power mode.

void enable_interrupts(): Enables global interrupts.

void disable_interrupts(): Disables global interrupts.

void TIMERA0_ON(unsigned int clk_counter): Starts Timer A0 with a specified counter value.

__interrupt void TimerA_ISR (void): Handles Timer A0 interrupts, used for timing operations.

__interrupt void Timer1_A0_ISR (void): Handles Timer 1 A0 interrupts, primarily for timing operations.

__interrupt void ADC10_ISR (void): Manages ADC10 interrupts, clearing flags and exiting low-power mode.

__interrupt void Joystick_handler(void): Handles joystick-related interrupts, managing the state machine for painter modes and calibration.

__interrupt void USCI0RX_ISR(void): Processes UART receive interrupts, handling various state transitions based on received data.

__interrupt void USCI0TX_ISR(void): Manages UART transmit interrupts, sending data and handling completion signals.

int divideBy360(long x): Performs division of a long integer by 360 using repeated subtraction.

uint32_t convert_hex_to_int(char *hex): Converts a hexadecimal string to a 32-bit integer.

int16_t degree(int16_t y_fp, int16_t x_fp): Computes the joystick angle in degrees using an efficient arctan calculation.

void timerA0Onscript(): Configures and starts Timer A0 for script timing, adjusting based on delay values.

void send_to_pc(): Sends data to the PC via UART, enabling the TX interrupt and managing timing.



- halGPIO.h - includes all functions and variables extern pointers for the system robastic operation.







							flash.c - Function Descriptions

Overview
The flash.c file contains functions for writing data to the Flash memory on the MSP430 microcontroller. These functions handle the storage of scripts and calibration data.

Functions
void ScriptData(void): Updates the size of the most recent file based on the length of the data stored in flash_buffer.

void write_Seg(void): Writes data from flash_buffer to the Flash memory, skipping newline characters, and locks the Flash segment afterward.

void write_SegC(int value1, int value2): Writes two integer values to specific Flash memory locations, commonly used to store calibration data.



- flash.h - includes all functions and variables extern pointers for the system robastic operation.








								BSP.c - Function Descriptions

Overview
The BSP.c file contains the board support package (BSP) functions, responsible for configuring and initializing various hardware components such as GPIOs, timers, UART, and ADC on the MSP430 microcontroller.

Functions
void GPIOconfig(void): Configures GPIO pins for the joystick, stepper motor, and LCD, and enables global interrupts.

void TIMER_A0_config(unsigned int clk_counter): Sets up Timer A0 for the stepper motor with a specified clock count, enabling interrupts.

void UART_init(void): Initializes the UART module for serial communication at 9600 baud using a 1 MHz clock.

void ADCconfig(void): Configures the ADC10 module to sample joystick inputs on channels A4 and A3, enabling interrupts and multiple conversions.

- bsp.h - includes all functions and variables extern pointers for the system robastic operation.






							Python Code Overview


This Python script is a comprehensive graphical user interface (GUI) application designed to control a motor-based system, interact with an MSP430 microcontroller, and manage a painting application via joystick input. It uses various libraries such as PySimpleGUI for the GUI, serial for serial communication, and Tkinter for handling the painting interface.

Functions
Paint.__init__: Initializes the painting application's main window, sets up buttons for "Back" and "Clear All," and configures a drawing canvas.

Paint.setup: Sets up the canvas for drawing, initializes the pen settings, and binds the mouse events for painting.

Paint.get_state_text: Returns a string representing the current painter state (Pen, Eraser, Neutral, or Unknown).

Paint.update_state_label: Updates the state label on the GUI to reflect the current painter state.

Paint.paint: Handles the drawing and erasing on the canvas based on the current painter state.

Paint.reset: Resets the old mouse coordinates when the mouse button is released.

Paint.clear_canvas: Clears all the drawings from the canvas.

Paint.close_painter: Closes the painting application and deactivates the painter mode.

GUI: Defines and manages the main GUI layout, handling user interactions for different modes such as manual control, painter mode, calibration, and script execution.

event_handler: Centralized event handler that processes user interactions with the GUI and triggers corresponding actions.

handle_manual_stepper: Manages the manual control mode of the motor-based system, allowing rotation, stopping, and joystick control.

handle_painter: Manages the painter mode where joystick input controls a virtual paintbrush.

handle_calibration: Manages the stepper motor calibration process and updates the GUI with calibration data.

handle_folder_selection: Handles the selection of a folder in the GUI and updates the file list.

handle_file_selection: Manages the selection and display of a file's content in the GUI.

handle_upload: Manages the process of uploading a script file to the MSP430 microcontroller and updates the GUI upon successful upload.

handle_exe_lst_sel: Handles the selection of a file from the execution list and returns the selected file name and its execution index.

handle_execute: Manages the execution of a selected script file by communicating with the MSP430, updating the GUI based on execution status.

show_window: Manages the visibility of different GUI layouts based on the current mode.

send_state: Sends a message to the MSP430 via serial communication, either as bytes or a string.

fetch_joystick_data: Retrieves and processes joystick data from the serial port.

translate_file: Translates the content of a file into a format suitable for the MSP430, converting commands and data into hexadecimal format.

main loop (if __name__ == '__main__'): Initializes the serial port for communication with the MSP430 and starts the main GUI loop.







The usre on PC-side will control the state FSM of the micro-processor. 