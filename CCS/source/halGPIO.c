    #include  "../header/halGPIO.h"     // private library - HAL layer
    #include  "../header/flash.h"     // private library - FLASH layer

// C libraries which we use
    #include "stdio.h"
    #include "stdint.h"
    #include "string.h"

    // global variables
    // Variables for timing
    unsigned int d = 50;
    unsigned int ms = 1309; // number of cycles for 10ms with smclk/3 in up mode

    // Variables for state 1
    int degree_steps = 5791; //steps per degree * 1000
    int steps_needed = 0;
    int joystick_steps = 0;
    int begin_manual_flag = 0;

    // Variables for state 2
    unsigned int joystick_location[2]; //joystick_location[0]=Vy , joystick_location[1]=Vx
    int Vx = 0;
    int Vy = 0;
    int joydegree = 0;
    int painter_state = 2;
    int send_coor = 1;

    // Variables for state 3
    int calibration_counter = 0;
    int calibflag = 0;
    int calib_start = 0;
    int steps_count = 2036;
    int phi = 88;
    int stepper_position = 0;  // the location of the stepper in steps
    int steps_count_flash = 2036;
    int calibrated = 0;

    // Variables for state 4
    char array_for_LCD[16];
    char *script_pointer1, *script_pointer2, *script_pointer3;
    int file_index = 5;
    char script_command[10], argument1[20], argument2[20];  // Buffers to hold instruction opcode and operands
    int erased_flash = 0;


    // Tx ISR variables
    int Tx_index = 0;    // int counter for transmission index
    char results_to_send[10];

    // Rx ISR variables
    int j=0;
    char RxData[60];
    char flash_buffer[60];




    //--------------------------------------------------------------------
    //             System Configuration
    //--------------------------------------------------------------------
    void sysConfig(void){
        // for running all the configurations needed for the system
        GPIOconfig();
        ADCconfig();
        lcd_init();
        UART_init();
    }
    //******************************************************************
    // Initialize the LCD
    //******************************************************************
    void lcd_init(){

        char init_value;

        if (LCD_MODE == FOURBIT_MODE) init_value = 0x3 << LCD_DATA_OFFSET;
            else init_value = 0x3F;

        LCD_RS_DIR(OUTPUT_PIN);
        LCD_EN_DIR(OUTPUT_PIN);
        LCD_RW_DIR(OUTPUT_PIN);
            LCD_DATA_DIR |= OUTPUT_DATA;
            LCD_RS(0);
        LCD_EN(0);
        LCD_RW(0);

        DelayMs(15);
            LCD_DATA_WRITE &= ~OUTPUT_DATA;
        LCD_DATA_WRITE |= init_value;
        lcd_strobe();
        DelayMs(5);
            LCD_DATA_WRITE &= ~OUTPUT_DATA;
        LCD_DATA_WRITE |= init_value;
        lcd_strobe();
        DelayUs(200);
            LCD_DATA_WRITE &= ~OUTPUT_DATA;
        LCD_DATA_WRITE |= init_value;
        lcd_strobe();

        if (LCD_MODE == FOURBIT_MODE){
            LCD_WAIT; // may check LCD busy flag, or just delay a little, depending on lcd.h
                    LCD_DATA_WRITE &= ~OUTPUT_DATA;
            LCD_DATA_WRITE |= 0x2 << LCD_DATA_OFFSET; // Set 4-bit mode
            lcd_strobe();
            lcd_cmd(0x28); // Function Set
        }
            else lcd_cmd(0x3C); // 8bit,two lines,5x10 dots

        lcd_cmd(0xF); //Display On, Cursor On, Cursor Blink
        lcd_cmd(0x1); //Display Clear
        lcd_cmd(0x6); //Entry Mode
        lcd_cmd(0x80); //Initialize DDRAM address to zero
        enable_interrupts();

    }

    //******************************************************************
    // send a command to the LCD
    //******************************************************************
    void lcd_cmd(unsigned char c){

        LCD_WAIT; // may check LCD busy flag, or just delay a little, depending on lcd.h

        if (LCD_MODE == FOURBIT_MODE)
        {
            LCD_DATA_WRITE &= ~OUTPUT_DATA;// clear bits before new write
                    LCD_DATA_WRITE |= ((c >> 4) & 0x0F) << LCD_DATA_OFFSET;
            lcd_strobe();
                    LCD_DATA_WRITE &= ~OUTPUT_DATA;
                LCD_DATA_WRITE |= (c & (0x0F)) << LCD_DATA_OFFSET;
            lcd_strobe();
        }
        else
        {
            LCD_DATA_WRITE = c;
            lcd_strobe();
        }
    }

    //******************************************************************
    // send data to the LCD
    //******************************************************************
    void lcd_data(unsigned char c){

        LCD_WAIT; // may check LCD busy flag, or just delay a little, depending on lcd.h

        LCD_DATA_WRITE &= ~OUTPUT_DATA;
        LCD_RS(1);
        if (LCD_MODE == FOURBIT_MODE)
        {
                LCD_DATA_WRITE &= ~OUTPUT_DATA;
                    LCD_DATA_WRITE |= ((c >> 4) & 0x0F) << LCD_DATA_OFFSET;
            lcd_strobe();
                    LCD_DATA_WRITE &= (0xF0 << LCD_DATA_OFFSET) | (0xF0 >> 8 - LCD_DATA_OFFSET);
                    LCD_DATA_WRITE &= ~OUTPUT_DATA;
            LCD_DATA_WRITE |= (c & 0x0F) << LCD_DATA_OFFSET;
            lcd_strobe();
        }
        else
        {
            LCD_DATA_WRITE = c;
            lcd_strobe();
        }

        LCD_RS(0);
    }

    //******************************************************************
    // LCD strobe functions
    //******************************************************************
    void lcd_strobe(){
      LCD_EN(1);
      asm("NOP");
      LCD_EN(0);
    }

    //******************************************************************
    // write a string of chars to the LCD
    //******************************************************************
    void lcd_puts(const char * s){

        while(*s)
            lcd_data(*s++);
    }


    // ------------------------------------------------------------------
    //                     Polling delays
    //---------------------------------------------------------------------
    //******************************************************************
    // Delay usec functions
    //******************************************************************
    void DelayUs(unsigned int cnt){

        unsigned char i;
        for(i=cnt ; i>0 ; i--) asm("nop"); // tha command asm("nop") takes raphly 1usec

    }
    //******************************************************************
    // Delay msec functions
    //******************************************************************
    void DelayMs(unsigned int cnt){

        unsigned char i;
        for(i=cnt ; i>0 ; i--) DelayUs(1000); // tha command asm("nop") takes raphly 1usec

    }
    //******************************************************************
    //            Polling based Delay function
    //******************************************************************
    void delay(unsigned int t){  //
        volatile unsigned int i;

        for(i=t; i>0; i--);
    }

    //---------------------------------------------------------------------
    //            Enter from LPM0 mode
    //---------------------------------------------------------------------
    void enterLPM(unsigned char LPM_level){
        if (LPM_level == 0x00)
          _BIS_SR(LPM0_bits);     /* Enter Low Power Mode 0 */
            else if(LPM_level == 0x01)
          _BIS_SR(LPM1_bits);     /* Enter Low Power Mode 1 */
            else if(LPM_level == 0x02)
          _BIS_SR(LPM2_bits);     /* Enter Low Power Mode 2 */
        else if(LPM_level == 0x03)
          _BIS_SR(LPM3_bits);     /* Enter Low Power Mode 3 */
            else if(LPM_level == 0x04)
          _BIS_SR(LPM4_bits);     /* Enter Low Power Mode 4 */
    }
    //---------------------------------------------------------------------
    //            Enable interrupts
    //---------------------------------------------------------------------
    void enable_interrupts(){
      _BIS_SR(GIE);
    }
    //---------------------------------------------------------------------
    //            Disable interrupts
    //---------------------------------------------------------------------
    void disable_interrupts(){
      _BIC_SR(GIE);
    }

    //---------------------------------------------------------------------
    //            Start Timer With counter
    //---------------------------------------------------------------------
    void TIMERA0_ON(unsigned int clk_counter){
        TIMER_A0_config(clk_counter);
        __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt
    }


    //---------------------------------------------------------------------
    //               Interrupt Services Routines
    //---------------------------------------------------------------------
    //*********************************************************************
    //                        TIMER A0 ISR
    //*********************************************************************

    #pragma vector = TIMER0_A0_VECTOR // For delay
    __interrupt void TimerA_ISR (void)
    {
        TACTL = MC_0; // stops the timer
        LPM0_EXIT;
    }

    //*********************************************************************
    //                        TIMER A ISR
    //*********************************************************************
    #pragma vector = TIMER1_A0_VECTOR // For delay
    __interrupt void Timer1_A0_ISR (void)
    {
        if(!TAIFG) {
        LPM0_EXIT;
        }
    }
    //*********************************************************************
    //            ADC10 Vector Interrupt Service Routine
    //*********************************************************************
    #pragma vector = ADC10_VECTOR
    __interrupt void ADC10_ISR (void)
    {
        __bic_SR_register_on_exit(CPUOFF);
        ADC10CTL0 &= ~ADC10IFG; // cancel ADC request
    }

//*********************************************************************
//            Port1 Interrupt Service Routine
//*********************************************************************
#pragma vector=PORT1_VECTOR
  __interrupt void Joystick_handler(void){
      IE2 &= ~UCA0TXIE;    // Disable USCI_A0 TX interrupt
      delay(debounceVal);  // debounce to avoid system problem
      if (state == state2 && (joy_stickIntPend & BIT5)) {
          switch(painter_state) {
              case 0:   // currently in paint mode, switch to erase mode
                  painter_state = 1;
                  Tx_index = 0;
                  strcpy(results_to_send, "");
                  strcpy(results_to_send, "E\n"); // send E to computer to change painter status to erase
                  IE2 |= UCA0TXIE; // Enable USCI_A0 TX interrupt
                  break;

              case 1:   // currently in erase mode, switch to neutral mode
                  painter_state = 2;
                  Tx_index = 0;
                  strcpy(results_to_send, "");
                  strcpy(results_to_send, "N\n"); // send N to computer to change painter status to neutral
                  IE2 |= UCA0TXIE; // Enable USCI_A0 TX interrupt
                  break;

              case 2:   // currently in neutral mode, switch to paint mode
                  painter_state = 0;
                  Tx_index = 0;
                  strcpy(results_to_send, "");
                  strcpy(results_to_send, "P\n"); // send P to computer to change painter status to paint
                  IE2 |= UCA0TXIE; // Enable USCI_A0 TX interrupt
                  break;
          }
          send_coor = 1; // continue to send joystick location
      }

      if (calibflag == 1 && (joy_stickIntPend & BIT5)){
          state = state3;
          calib_start = calib_start^1; // change the calibration flag to 1 when calibrating and 0 when we press another time
          LPM0_EXIT;

      }
      joy_stickIntPend &= ~BIT5; // disable joystick interrupt request

      LPM0_EXIT;
}


    //***********************************RX ISR******************************************
    #if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
    #pragma vector=USCIAB0RX_VECTOR
    __interrupt void USCI0RX_ISR(void)
    #elif defined(__GNUC__)
    void __attribute__ ((interrupt(USCIAB0RX_VECTOR))) USCI0RX_ISR (void)
    #else
    #error Compiler not supported!
    #endif
    {
        RxData[j++] = UCA0RXBUF;

        // RX interrupts for script mode - state 4
        if (script_state == nameupdate && RxData[j-1] == '\x0a') {   //  indicates EOF
            int i;
            for (i = 0; i < j; i++) {
                file.file_name[i] = RxData[i]; // Copy the received file name from RxData to file.file_name
            }
            script_state = scriptreceive; // file received
            j = 0;
        }
        if (script_state == scriptreceive && RxData[j-1] == 'f') {
            strcpy(flash_buffer, RxData);  // upload the received data to the flash input buffer
            script_state = scriptupload; // Set script state to indicate flash upload of file
            script_pointer1 = (char*) 0x1000; // Set pointer to the address for the 1st file
            file.file_ptr[0] = script_pointer1; // Assign pointer to the file_ptr array
            file.num_of_files += 1; // Update the number of files
            j = 0; // Reset the index for RxData
            state = state4;
        }
        if (script_state == scriptreceive && RxData[j-1] == 'g') { // Pointer for the 2nd selected file
            strcpy(flash_buffer, RxData); // upload the received data to the flash input buffer
            script_state = scriptupload; // Set script state to indicate flash upload of file
            script_pointer2 = (char*) 0x1040; // Set pointer to the address for the 2nd file
            file.file_ptr[1] = script_pointer2; // Assign pointer to the file_ptr array
            file.num_of_files += 1; // Update the number of files
            j = 0; // Reset the index for RxData
            state = state4;
        }
        if (script_state == scriptreceive && RxData[j-1] == 'h') { // Pointer for the 3rd selected file
            strcpy(flash_buffer, RxData); // upload the received data to the flash input buffer
            script_state = scriptupload; // Set script state to indicate flash upload of file
            script_pointer3 = (char*) 0x1080; // Set pointer to the address for the 3rd file
            file.file_ptr[2] = script_pointer3; // Assign pointer to the file_ptr array
            file.num_of_files += 1; // Update the number of files
            j = 0; // Reset the index for RxData
            state = state4;
        }

        // Check if the received character is 'F', 'G', or 'H' to set the execution index
        if (RxData[j-1] == 'F') { // Index for the executed list (1 file)
            script_state = executescript; // Set flag to indicate that execution should begin
            state = state4; // Set the system state to script mode
            memset(RxData, 0, 60); // reset receiver buffer
            j = 0; // Reset the index for RxData
            file_index = 0; // set the file index accordingly

        }
        if (RxData[j-1] == 'G') { // Index for the executed list (2 files)
            script_state = executescript; // Set flag to indicate that execution should begin
            state = state4; // Set the system state to script mode
            memset(RxData, 0, 60); // reset receiver buffer
            j = 0; // Reset the index for RxData
            file_index = 1; // set the file index accordingly
        }
        if (RxData[j-1] == 'H') { // Index for the executed list (3 files)
            script_state = executescript; // Set flag to indicate that execution should begin
            state = state4; // Set the system state to script mode
            memset(RxData, 0, 60); // reset receiver buffer
            j = 0; // Reset the index for RxData
            file_index = 2; // set the file index accordingly
        }
        else if (RxData[0] == 's' && script_state != nameupdate && script_state != scriptreceive) { // Command to set state to 'script mode'
            state = state4; // Set the system state to script mode
            script_state = nameupdate;
            j = 0; // Reset the index for RxData
        }

        // RX interrupts for script mode - state 1
        // Check for state commands to update the state machine
        if (RxData[0] == 'm') { // Command to set state to 'manual'
            state = state1; // Set the system state to manual
            stepper_status = stopMode; // Initialize the stepping state
            j = 0; // Reset the index for RxData
        }
        // Check for commands to control motor rotation
        if (RxData[0] == 'A') { // Command to auto-rotate
            state = state1; // Set the system state to manual
            stepper_status = rotationMode; // Set the stepping state for auto-rotation
            j = 0; // Reset the index for RxData
        }
        else if (RxData[0] == 'M') { // Command to stop rotation
            state = state1; // Set the system state to manual
            stepper_position = 0 ; // re-initialize stepper position
            stepper_status = stopMode; // Set the stepping state to stop rotation
            j = 0; // Reset the index for RxData
        }
        else if (RxData[0] == 'J') { // Command to rotate with joystick
            state = state1; // Set the system state to manual
            stepper_status = manualMode; // Set the stepping state to joystick control
            stepper_position = 0;
            begin_manual_flag = 0;
            j = 0; // Reset the index for RxData
        }

        // RX interrupts for script mode - state 2
        else if (RxData[0] == 'P') { // Command to set state to 'painter'
            state = state2; // Set the system state to painter mode
            j = 0; // Reset the index for RxData
        }
        else if (RxData[0] == 'R'){ // PC side is ready to receive data
            state = state2;         // make sure state is painter
            send_coor = 1;
            j = 0;
        }

        // RX interrupts for script mode - state 3
        else if (RxData[0] == 'C') { // Command to set state to 'calibration'
            state = state3; // Set the system state to calibration mode
            calibflag = 1;
            j = 0; // Reset the index for RxData
        }

        // RX general interrupts for any "back" on PC
        else if (RxData[0] == 'B') {
            if (calibflag == 1){
                write_SegC (steps_count_flash, phi); // write phi and total steps for flash
                calibrated = 1;  // mark that we done calibrating and we can work with the recent results
            }
            state = state0; // Set the system state to calibration mode
            calibflag = 0;
            calib_start = 0;    // to exit calibration loop
            painter_state = 0;
            send_coor = 0;
            j = 0; // Reset the index for RxData
        }

        // Exit Low Power Mode 0 and enable interrupts
        LPM0_EXIT;

    }


//***********************************TX ISR******************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0TX_VECTOR))) USCI0TX_ISR (void)
#else
#error Compiler not supported!
#endif
{
  UCA0TXBUF = results_to_send[Tx_index];
  Tx_index++;
  if (results_to_send[Tx_index -1] == '\n'){         // TX over?
      Tx_index = 0;
      strcpy(results_to_send, ""); // erase the buffer array
      IE2 &= ~UCA0TXIE;            // Disable USCI_A0 TX interrupt
      LPM0_EXIT;
      IE2 |= UCA0RXIE;
  }
}

    //----------------------------------------------------------------------
    //             Stepper Motor help functions
    //----------------------------------------------------------------------
    int divideBy360(long x) {
        int quotient = 0;
        // Multiply x by 1024 using bitwise shift
        x = x * 1000; // equivalent to x * 1024
        // Repeated subtraction to calculate integer division x / 360
        while (x >= 360) {
            x -= 360;
            quotient++;
        }

        return quotient;
    }

    //---------------------------------------------------------------------
    //            General Functions
    //---------------------------------------------------------------------
    //-----------------------------------------------------------------------
    uint32_t convert_hex_to_int(char *hex) {
        uint32_t val = 0;  // Initialize the result value to 0
        int l;

        // Loop through the first 2 characters of the hexadecimal string
        for(l = 0; l < 2; l++) {
            // Get the current character from the string and move the pointer to the next character
            uint8_t byte = *hex++;

            // Convert the character to its corresponding 4-bit value
            if (byte >= '0' && byte <= '9') {
                byte = byte - '0';  // Convert '0'-'9' to 0-9
            } else if (byte >= 'a' && byte <= 'f') {
                byte = byte - 'a' + 10;  // Convert 'a'-'f' to 10-15
            } else if (byte >= 'A' && byte <= 'F') {
                byte = byte - 'A' + 10;  // Convert 'A'-'F' to 10-15
            }

            // Shift the accumulated value left by 4 bits to make space for the new 4-bit digit
            // Then, add the new 4-bit value to the result
            val = (val << 4) | (byte & 0xF);
        }

        // Return the final 32-bit integer value
        return val;
    }
/*
    int division(int dividend, int divisor) {
        /* division function to create a division between 2 integers with the result multiplied by 1000 in order to get the result as integer */
/*        int quotient = 0;           // Initialize quotient to store the final result
        int multiplier = 1000;      // Define a multiplier to scale up the dividend to avoid fractions

        dividend *= multiplier;     // Scale up the dividend by the multiplier to handle integer division

        // Loop until the dividend is smaller than the divisor
        while (dividend >= divisor) {
            int temp_divisor = divisor;   // Create a temporary divisor for shifting operations
            int temp_quotient = 1;        // Temporary quotient initialized to 1 for the shifting process

            // Shift temp_divisor left until it's larger than the dividend
            while ((temp_divisor << 1) <= dividend) {
                temp_divisor <<= 1;      // Shift temp_divisor left (equivalent to multiplying by 2)
                temp_quotient <<= 1;     // Shift temp_quotient left (equivalent to multiplying by 2)
            }

            dividend -= temp_divisor;    // Subtract the largest shifted divisor from the dividend
            quotient += temp_quotient;   // Add the corresponding quotient to the final result
        }

        return quotient;                 // Return the final quotient, which is dividend/divisor * 1000
    }*/
    //-------------------ATAN2- Fixed point - returns degrees---------------------------
    int16_t degree(int16_t y_fp, int16_t x_fp)
    // this function returns the joystick degree by calculating the arctan of Vy/Vx based on taylor series for efficient calculation
    {
        int32_t coeff_1 = 45;
        int32_t coeff_1b = -56; // 56.24;
        int32_t coeff_1c = 11;  // 11.25
        int16_t coeff_2 = 135;

        int16_t angle = 0;

        int32_t r;
        int32_t r3;

        int16_t y_abs_fp = y_fp;
        if (y_abs_fp < 0)
            y_abs_fp = -y_abs_fp;

        if (y_fp == 0)
        {
            if (x_fp >= 0)
            {
                angle = 0;
            }
            else
            {
                angle = 180;
            }
        }
        else if (x_fp >= 0)
        {
            r = (((int32_t)(x_fp - y_abs_fp)) << 15) /((int32_t)(x_fp + y_abs_fp));

            r3 = r * r;
            r3 =  r3 >> 15;
            r3 *= r;
            r3 =  r3 >> 15;
            r3 *= coeff_1c;
            angle = (int16_t) (coeff_1 + ((coeff_1b * r + r3) >> 15));
        }
        else
        {
            r = (((int32_t)(x_fp + y_abs_fp)) << 15) /((int32_t)(y_abs_fp - x_fp));
            r3 = r * r;
            r3 =  r3 >> 15;
            r3 *= r;
            r3 =  r3 >> 15;
            r3 *= coeff_1c;
            angle = coeff_2 + ((int16_t)(((coeff_1b * r + r3) >>15)));
        }

        if (y_fp < 0)
            return (360-angle);     // negate if in quad III or IV
        else
            return (angle);
    }


//---------------------------------------------------------------------
//            timerA0On - script
//---------------------------------------------------------------------
/*this function activates timer A0 according to the value of x in ms.
 the calculation of the value set in TA0CCR0 is the following:
 1 ms in SMCLK clock cycle is given by: 0xffff/500 = 131
 since there is no hardware multiplier, the multiplication is done by rotating the
 x value left 7 times, which is the same as performing x*128.
 the value of  the previuos x is then added 3 times in order to complete the calculation:
 TA0CCR0 = 128x + 3x = 131x = (0xffff/500)*x = x [ms]
 */
void timerA0Onscript(){
  int temp = d;
  while(temp >50){
      TIMERA0_ON(0xffff);
      temp -= 50;
  }
  if (temp != 0){
      int temp2 = temp * ms;
      TIMERA0_ON(temp2);
  }
}

void send_to_pc(){
    Tx_index = 0;
    IE2 |= UCA0TXIE; // Enable USCI_A0 TX interrupt
    __bis_SR_register(LPM0_bits + GIE);          // LPM0, Tx_ISR will force exit
    TIMERA0_ON(10000);
}
