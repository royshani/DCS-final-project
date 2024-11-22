#include  "../header/api.h"         // private library - API layer
#include  "../header/halGPIO.h"     // private library - HAL layer
#include  "../header/flash.h"     // private library - FLASH layer

// C libraries which we use
#include "stdio.h"
#include "math.h"
#include "string.h"




//---------------------------------------------------
//              state1 Function - manual stepper motor
//---------------------------------------------------
void stepper_control(){
    // this function controls the stepper motor and rotate between 3 states based on user decisions
    while (state == state1){
        switch(stepper_status){
            case rotationMode: // in this state the user rotate the stepper motor until it reaches the black line for calibration
                while (stepper_status == rotationMode){
                    clockwise_step(1500); // rotate the stepper motor constantly in 22HZ until the user press to stop
                }

            case stopMode:
                __bis_SR_register(LPM0_bits + GIE); // stop the stepper motor move and set the stepper position to 0

            case manualMode:
                    Manual_stepper(); // operating manual control of the stepper with the Joystick
         }
    }
}

void Manual_stepper(){
    // the main idea with this function is to implement a manual control of the stepper using the joystick, the operation from is
    // sampling the joystick position (Vx, Vy) and check the pointing angle, then we calculate the steps needed to be done by the
    // stepper motor (by our calibration results) in the shortest way toward the desired angle
    while (stepper_status == manualMode && state == state1){
        JoyStick_Sample();                  // this function returns the values of Vx, Vy
        if (!(Vx >= 200 && Vx <= 650 && Vy >= 200 && Vy <= 650)) {
            // Enter the loop or perform actions if Vx or Vy is not in the range 200 to 650
            int prev_degree;
            prev_degree = joydegree;
            int tempVx = Vx - 481;
            int tempVy = 474 - Vy;
            joydegree = degree(tempVy, tempVx);   // this function returns the value of the joystick location in degrees
                    go_to_degree(joydegree);      // this function tell the stepper motor which way to go and how many steps
            }
        }
        }

void go_to_degree(int degree) {
    ADC10CTL0 &= ~ENC;                           // turn off ADC activity

    int k;
    if (calibrated == 1){
        degree_steps = divideBy360(steps_count_flash);
    }
    else{
        degree_steps = 5791; // default value
    }

    long joystick_steps = (long)degree_steps * (long)degree;  // Use 'long' for larger range
    joystick_steps >>= 10;  // Shift right by 10 to scale down

    long steps_needed=0;
    if (stepper_position >= joystick_steps) {
        steps_needed = steps_count_flash + joystick_steps - stepper_position;
    } else {
        steps_needed = joystick_steps - stepper_position;
    }

    steps_needed = steps_needed >> 2;  // Divide by 4 to get the correct step count
    if (steps_needed < 255){
        for (k = 0; k < steps_needed; k++) {
            counter_clockwise_step(500); // 65 HZ
            stepper_position += 4;
            if (stepper_position > steps_count_flash) {
                stepper_position -= steps_count_flash;
            }
        }
    }
    if (steps_needed >= 255){
        steps_needed = 510 - steps_needed;
        for (k = 0; k < steps_needed; k++) {
            clockwise_step(500); // 65 HZ
            stepper_position -= 4;
            if (stepper_position < 0) {
                stepper_position += steps_count_flash;
            }
        }
    }
    ADC10CTL0 |= ENC;
}

//---------------------------------------------------
//              state2 Functions - Painter
//---------------------------------------------------
void Leonardo_de_joystick() {
    // this function sample the Vx and Vy location and send to the PC to change the mouse location accordingly
    while (state == state2){
        joy_stickIntEN &= ~BIT5; // disable joystick interrupts
        Tx_index = 0;
        if (send_coor == 1){ // this is a flag indicates the PC is ready to receive another sample
            JoyStick_Sample(); // samplr joystick location
            sprintf(results_to_send, "%d,%d\n", Vx, Vy); // Convert Vx and Vy to strings and concatenate with a comma in between
            IE2 |= UCA0TXIE; // Enable USCI_A0 TX interrupt
            send_coor = 0; // reset synchronization flag to wait for the computer to be ready
            __bis_SR_register(LPM0_bits + GIE);          // LPM0, TX_ISR will force exit
        }
        joy_stickIntEN |= BIT5; // enable joystick interrupts for painter state changes
    }
}
//-------------------------------------------------------------
//                Joystick_sample_for_paint
//-------------------------------------------------------------
void JoyStick_Sample(){
    // this function sample 4 times the Vx, Vy location and divide them by 4 in order to reduce noise and get the average location
    Vx = 0;
    Vy = 0;
    int i;
    for (i = 0 ; i < 4 ; i++){                       // sample for 4 time to get better resolution
        ADC10CTL0 &= ~ENC;                           // turn off ADC activity
        while (ADC10CTL1 & ADC10BUSY);               // Wait for ADC10 core to be active
        ADC10SA = (unsigned int)&joystick_location[0]; // assign ADC data address to joystick_location which is an array with the dimension of 2
        ADC10CTL0 |= ENC + ADC10SC;                  //  start sampling and conversion
        __bis_SR_register(LPM0_bits + GIE);          // LPM0, ADC10_ISR will force exit
        Vy += joystick_location[0];                  // add the Vy sample to the Vy accumulator
        Vx += joystick_location[1];                  // add the Vx sample to the Vx accumulator
    }
    Vy = Vy>>2;         // divide by 4 to get the average Joystick location in Y - axis
    Vx = Vx>>2;         // divide by 4 to get the average Joystick location in X - axis
}

//---------------------------------------------------
//              state3 Function - calibration
//---------------------------------------------------
void StepperCalibration(int steps_count){
    /*when getting a start order from the user, the stepper motor start spinning clockwise. The function count each step until
    getting a stop order from the user, then the function exits the loop and divide the number of step by 360 arithmetically
    in order to get the step angle*/
    steps_count_flash = 0;
    while (state == state3){
        steps_count = 0; // initialize stepper location
        while(calib_start==1){  // flag to indicate we still in calibration mode
            clockwise_step(1500); // rotate in 22HZ to allow the user to stop the stepper when the pointer reaches the black line
            (steps_count)++; // count stepper moves
            steps_count_flash = steps_count; // saves the information in a non volatile variable to move it to flash and use in other functions
            if (calib_start==0){
                steps_count_flash = steps_count_flash << 2; //calculation to get 4 steps per loop
                int temp_step_num = steps_count_flash;
                temp_step_num = temp_step_num >> 6; // divide by 64 which is the numerber of the gearbox reduction inside the motor 64:1
                phi = divideBy360(temp_step_num);   // divide by 360 to get phi per motor spin
                send_calibration(steps_count_flash, phi); // send our results to PC
                state = state0;
            }

        }

    }

}
//---------------------------------------------------
//              stepper motor functions
//---------------------------------------------------
void counter_clockwise_step(int stepper_period){
    SteppperOUT = BIT0;
    TIMERA0_ON(stepper_period);
    SteppperOUT = BIT3;
    TIMERA0_ON(stepper_period);
    SteppperOUT = BIT2;
    TIMERA0_ON(stepper_period);
    SteppperOUT = BIT1;
    TIMERA0_ON(stepper_period);
}

void clockwise_step(int stepper_period){
    SteppperOUT = BIT3;
    TIMERA0_ON(stepper_period);
    SteppperOUT = BIT0;
    TIMERA0_ON(stepper_period);
    SteppperOUT = BIT1;
    TIMERA0_ON(stepper_period);
    SteppperOUT = BIT2;
    TIMERA0_ON(stepper_period);
}

void send_calibration(int to_send1, int to_send2){
    // Convert to_send1 and to_send2 to strings and concatenate with a comma in between
    sprintf(results_to_send, "%d,%d\n",to_send1, to_send2);
    // Initialize the transmission index
    Tx_index = 0;
    // Enable the USCI_A0 TX interrupt to handle subsequent bytes of data
    IE2 |= UCA0TXIE;
}


//-----------------------------------------------------
//              state 4 - scripts
//----------------------------------------------------
void script_mode(){
    // this function handles the scripts upload
    while (state == state4){
        switch (script_state){
            case(scriptupload):
                FCTL2 = FWKEY + FSSEL0 + FN1; // calibrate internal clock for flash operation
                ScriptData(); // find the data size
                write_Seg(); // write the script to the FLASH memory
                strcpy(results_to_send, ""); // reset the transmit string
                strcpy(results_to_send, "R\n"); // Copy the string into the array
                IE2 |= UCA0TXIE; // Enable USCI_A0 TX interrupt
                __bis_SR_register(LPM0_bits + GIE);
                script_state = nameupdate; // prepare to receive file name
                state = state0; // go to idle state for low power consumption
                break;
            case(executescript):
                execute_file();
                break;
        }
    }
}
//---------------Execute Script--------------------------------
void execute_file(void)
// this function runs the script task by task, it switches between states base on first argument in each line
// and takes the second instruction in line as the argument
{
    char *script_pointer;  // Pointer to navigate through the flash memory segment
// Initialize variables
    int argument_int;
    int y = 0;
    char argument_char[4];
/*    switch (file_index){
        case 0:
            script_pointer = (char*) 0x1000;
            break;
        case 1:
            script_pointer = (char*)  0x1040;
            break;
        case 2:
            script_pointer = (char*)  0x1080;
            break;
    }*/
    script_pointer = file.file_ptr[file_index];  // Point to the script data in flash memory

    // Process up to 64 bytes of script data
    while (y<60 && state ==state4)
    {
        // Read the opcode (operation code) from the flash memory first byte is zero second will be the opcode from the assignment description
        script_command[0] = *script_pointer++;
        script_command[1] = *script_pointer++;
        y = y + 2;  // Increment the counter by 2 for opcode bytes

        // Choose which operation to execute based on the received opcode
        switch (script_command[1])
        {
        case '1':  // Opcode '1' = count-up operation
            lcd_clear();  // Clear the LCD display
            // Read the operand for the count-up operation
            argument1[0] = *script_pointer++;
            argument1[1] = *script_pointer++;
            y = y + 2;  // Increment the counter by 2 for operand bytes
            argument_int = convert_hex_to_int(argument1);  // Convert operand from hex to integer
            // Perform count-up operation
            lcd_clear();  // Clear LCD display
            lcd_home();   // Set LCD cursor to home position

            strcpy(array_for_LCD, "0");
            lcd_puts(array_for_LCD);  // Display '0' on LCD
            lcd_home();   // Set LCD cursor to home position again
            argument_int++;
            countUP(argument_int);  // Perform count-up operation
            break;

        case '2':  // Opcode '2' = count-down operation
            lcd_clear();  // Clear the LCD display
            // Read the operand for the count-down operation
            argument1[0] = *script_pointer++;
            argument1[1] = *script_pointer++;
            y = y + 2;  // Increment the counter by 2 for operand bytes
            argument_int = convert_hex_to_int(argument1);  // Convert operand from hex to integer
            countDOWN(argument_int);  // Perform count-down operation
            break;

        case '3':  // Opcode '3' = rotate-right operation
            lcd_clear();  // Clear the LCD display
            // Read the operand for the rotate-right operation
            argument1[0] = *script_pointer++;
            argument1[1] = *script_pointer++;
            y = y + 2;  // Increment the counter by 2 for operand bytes
            argument_int = convert_hex_to_int(argument1);  // Convert operand from hex to integer
            // Perform rotate-right operation
            rra_LCD(argument_int);  // Perform rotate-right on LCD
            break;

        case '4':  // Opcode '4' = setting a new delay time
            lcd_clear();  // Clear the LCD display
            // Read the operand for delay time
            argument1[0] = *script_pointer++;
            argument1[1] = *script_pointer++;
            y = y + 2;  // Increment the counter by 2 for operand bytes
            argument_int = convert_hex_to_int(argument1);  // Convert operand from hex to integer
            d = argument_int;  // Convert delay time to milliseconds (multiplying by ms)
            break;

        case '5':  // Opcode '5' = resetting the LCD count
            lcd_clear();  // Clear the LCD display
            break;

        case '6':  // Opcode '6' = pointing the stepper motor to a specific degree
            lcd_clear();  // Clear the LCD display
            // Read the operand for motor position
            argument1[0] = *script_pointer++;
            argument1[1] = *script_pointer++;
            y = y + 2;  // Increment the counter by 2 for operand bytes
            argument_int = convert_hex_to_int(argument1);  // Convert operand from hex to integer
            lcd_clear();  // Clear the LCD display
            go_to_degree_exe(argument_int);  // Move motor to specified position
            TIMERA0_ON(10000);
            int k =0;
            while (k< 10){ // loop for synchronization with PC to announce end of operation
                memset(results_to_send, 0, 10);
                strcpy(results_to_send, "R\n");
                send_to_pc();
                k++;
            }
            break;

        case '7':  // Opcode '7' = scanning an area between two angles
            lcd_clear();  // Clear the LCD display
            // Read the start angle for scanning
            argument1[0] = *script_pointer++;
            argument1[1] = *script_pointer++;
            y = y + 2;  // Increment the counter by 2 for operand bytes
            argument_int = convert_hex_to_int(argument1);  // Convert operand from hex to integer
            go_to_degree(argument_int);  // Move motor to specified position
            sprintf(array_for_LCD, "%d",argument_int);
            lcd_puts(array_for_LCD);
            TIMERA0_ON(0xFFFF); // wait 0.5 sec to allow the user to watch the LCD
            // Read the stop angle for scanning
            argument2[0] = *script_pointer++;
            argument2[1] = *script_pointer++;
            y = y + 2;  // Increment the counter by 2 for second operand bytes
            argument_int = convert_hex_to_int(argument2);  // Convert operand from hex to integer
            go_to_degree(argument_int);  // Move motor to specified position
            TIMERA0_ON(0xFFFF); // wait 0.5 sec to allow the user to watch the LCD
            sprintf(array_for_LCD, "%d",argument_int);
            lcd_clear();
            lcd_puts(array_for_LCD);
            TIMERA0_ON(0xFFFF);
            break;

        case '8':  // Opcode '8' = sleep operation
            enable_interrupts();
            enterLPM(mode0);
            break;
        }
    }
}



void go_to_degree_exe(int degree) {
    int k =0;
    if (calibrated == 1){
        degree_steps = divideBy360(steps_count_flash);
    }
    else{
        degree_steps = 5791; // default value
    }
    long joystick_steps = (long)degree_steps * (long)degree;  // Use 'long' for larger range
    joystick_steps >>= 10;  // Shift right by 10 to scale down

    long steps_needed=0;
    if (stepper_position >= joystick_steps) {
        steps_needed = steps_count_flash + joystick_steps - stepper_position;
    } else {
        steps_needed = joystick_steps - stepper_position;
    }

    steps_needed = steps_needed >> 2;  // Divide by 4 to get the correct step count
    if (steps_needed < 255){

        while (k <=steps_needed){
            k++;
            counter_clockwise_step(500); // 65 HZ
            stepper_position += 4;
            TIMERA0_ON(10000);
            if (stepper_position > steps_count_flash) {
                stepper_position -= steps_count_flash;
            }
            memset(results_to_send, 0, 10);
            sprintf(results_to_send, "%d\n", stepper_position);
            send_to_pc();
        }

    }
    if (steps_needed >= 255){
        steps_needed = 510 - steps_needed;
        while (k <= steps_needed ){
            k++;
            clockwise_step(500); // 65 HZ
            stepper_position -= 4;
            TIMERA0_ON(10000);
            if (stepper_position < 0) {
                stepper_position += steps_count_flash;
            }
            memset(results_to_send, 0, 10);
            sprintf(results_to_send, "%d\n", stepper_position);
            send_to_pc();

        }

    }
}
//********************************************************************
//           scripts general functions
//********************************************************************
//---------------------------------------------------------------------
//            countUP
//---------------------------------------------------------------------
void countUP(int argument_int){
    // function to count from 0 to the argument with d delay
    int temp1 = 0;
    lcd_home();   // Move the cursor to the home position
    cursor_off;   // Turn off the cursor
    // Convert integer countUp to string and display on LCD
    while (argument_int > 0){
        sprintf(array_for_LCD, "%d",temp1);
        lcd_puts(array_for_LCD);
        temp1++;// Increment countUp
        argument_int--;
        timerA0Onscript();
        lcd_home();   // Move the cursor to the home position
        }
   }
//---------------------------------------------------------------------
//            countDOWN
//---------------------------------------------------------------------

void countDOWN(int argument_int){
    // function to count down from the argument to the argument the argument with d delay
    lcd_home();   // Move the cursor to the home position
    cursor_off;   // Turn off the cursor
    // Convert integer countDOWN to string and display on LCD
    while (argument_int >= 0){
        sprintf(array_for_LCD, "%d",argument_int);
        lcd_puts(array_for_LCD);
        argument_int--;// decrement count
        timerA0Onscript();
        lcd_home();   // Move the cursor to the home position
        }
   }

//---------------------------------------------------------------------
//            rra_LCD
//---------------------------------------------------------------------

void rra_LCD(int rotate_char){
    lcd_clear();  // Clear the LCD display
    lcd_home();   // Move the cursor to the home position
    cursor_off;   // Turn off the cursor
    if (rotate_char >= 32 && rotate_char <= 126) {
        // If rotate_char is a printable ASCII character, add it as a character
        sprintf(array_for_LCD, "%c", rotate_char);
    } else {
        // Otherwise, add it as an integer
        sprintf(array_for_LCD, "%d", rotate_char);
    }

    int k;
    int i;
    for (k=0 ; k <= 15 ; k++){
        lcd_puts(array_for_LCD);
        timerA0Onscript();
        lcd_clear();  // Clear the LCD display
        lcd_home();   // Move the cursor to the home position
        for (i=0 ; i <= k ; i++){
            lcd_cursor_right();
        }
    }
    for (k=0 ; k <= 15 ; k++){
        lcd_clear();  // Clear the LCD display
        lcd_home();   // Move the cursor to the home position
        lcd_new_line;
        for (i=0 ; i < k ; i++){
            lcd_cursor_right();
        }
        lcd_puts(array_for_LCD);
        timerA0Onscript();
    }
}
