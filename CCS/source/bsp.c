#include  "../header/bsp.h"    // private library - BSP layer

//-----------------------------------------------------------------------------
//           GPIO configuration
//-----------------------------------------------------------------------------
void GPIOconfig(void){

  WDTCTL = WDTHOLD | WDTPW;     // Stop WDT

    // JoyStick Configuration
    /*P1.3 - Vrx; P1.4 - Vry; P1.5 - PB*/
    /* According to the data Sheet For A3,A4 ADC input Selection
     *  we need to select nothing for the corresponding pins on the SEL, DIR PORTS (Don't Cares)*/
    joy_stickSEL &= ~BIT5;  // P1.5 = I/O
    joy_stickDIR &= ~BIT5;  // P1.5 = Input
    joy_stickOUT |= BIT5;  // clear P1.5 output
    joy_stickIntEN |= BIT5; // Enable P1.5 interrupts
    joy_stickIntEdgeSel |= BIT5; // P1.5 active for pull-up & pull-down
    joy_stickREN |= BIT5;
    joy_stickIntPend &= ~BIT5; // Reset P1.5 Interrupt flag

    // Stepper Motor Configuration
    StepperSEL &= ~(BIT0+BIT1+BIT2+BIT3);  // P2.0-P2.3 Sel = I/O
    StepperDIR |= BIT0+BIT1+BIT2+BIT3;  // P2.0-P2.3 output = Output

    // LCD Setup
    LCD_DATA_WRITE &= ~0xFF;      // Clear LCD data output
    LCD_DATA_DIR |= 0xF0;         // Set LCD data pins as outputs (upper 4 bits)
    LCD_DATA_SEL &= ~0xF0;        // Select GPIO function for LCD data
    LCD_CTL_SEL &= ~0xC1;         // Select GPIO function for LCD control


    _BIS_SR(GIE);                     // enable interrupts globally
  }

  //-------------------------------------------------------------------------------------
  //            Timer A  configuration
  //-------------------------------------------------------------------------------------
  void TIMER_A0_config(unsigned int clk_counter){
      TACCR0 = clk_counter; // (2^20/8)*655 = 50HZ/4 which is 50 HZ for a steppermotor step
      TACCTL0 = CCIE; // enable TACCR0 interrupts
      TA0CTL = TASSEL_2 + MC_1 + ID_3;  // clock source = SMCLK, operation mode = Up-mode, devision by 2^3=8
      TA0CTL |= TACLR; //clear clock counter for initialization
  }


  //-------------------------------------------------------------------------------------
  //                              UART init
  //-------------------------------------------------------------------------------------
  void UART_init(void) {
      if (CALBC1_1MHZ==0xFF)                  // If calibration constant erased
        {
          while(1);                               // do not load, trap CPU!!
        }

      // Set DCO to 1 MHz
      DCOCTL = 0;                // Reset DCO control register
      BCSCTL1 = CALBC1_1MHZ;     // Set DCO to 1 MHz using the calibration constant
      DCOCTL = CALDCO_1MHZ;      // Load DCO control register with calibrated value

      // Configure UART
      UCA0CTL1 |= UCSSEL_2;      // Use SMCLK as the clock source for UART
      UCA0BR0 = 104;             // Set baud rate to 9600 with a 1 MHz clock (N = 1,000,000 / 9600 ≈ 104)
      UCA0BR1 = 0x00;            // High byte of the baud rate setting
      UCA0MCTL = UCBRS0;         // Set modulation for UART communication according to the family user guide table (p.480)

      // Initialize USCI state machine
      UCA0CTL1 &= ~UCSWRST;      // Clear the software reset to start the USCI state machine

      // Select P1.1 for RXD and P1.2 for TXD
      P1SEL |= BIT1 + BIT2;      // P1.1 = RXD, P1.2 = TXD
      P1SEL2 |= BIT1 + BIT2;     // Further configuration of P1.1 and P1.2

  }


  //-------------------------------------------------------------------------------------
  //            ADC configuration
  //-------------------------------------------------------------------------------------
  void ADCconfig(void){

      ADC10CTL0 = ADC10SHT_3 + MSC + ADC10ON + ADC10IE;  // Activate ADC10, enable interrupts, sample and hold time is /**64 ADC10CLK cycles**/
      //(and another 13 cycles for conversion , and choose MSE state as explained below
      /* MSE = The first rising edge of the SHI signal triggers the sampling timer,
      but further sample-and-conversions are performed automatically as soon
      as the prior conversion is completed*/

      ADC10CTL1 = INCH_4 + CONSEQ_1 + ADC10SSEL_3; /* Choosing A4 & A3 channels  for conversion of the analog input using decrement
      channels capturing starting from A4 (y axis) for 1 repetition each time */

      ADC10AE0 |= BIT4 + BIT3;   // P1.3-4 ADC10 option select

      ADC10DTC1 = 0x02;   // 2 conversions
      /** check for 4 conversion option**/
  }



