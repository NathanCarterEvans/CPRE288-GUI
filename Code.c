lcd.c:
/**
 * lcd.c: Functions for displaying content on the 4x16 Character LCD Screen.
 * Updated on 8/22/19 for compatibility with Isaac Rex's timer fixes.  (date corrected by phjones)
 *
 *  @author Noah Bergman, Eric Middleton
 *  @date 02/29/2016
 *
 *
 */


#include "lcd.h"

#define BIT0		0x01
#define BIT1		0x02
#define BIT2		0x04
#define BIT3		0x08
#define BIT4		0x10
#define BIT5		0x20
#define BIT6		0x40
#define BIT7		0x80


//Defines for LCD Control Commands
#define HD_LCD_CLEAR 		0x01
#define HD_RETURN_HOME		0X02

#define HD_CURSOR_SHIFT_DEC	0X05
#define HD_CURSOR_SHIFT_INC	0X07
#define HD_DISPLAY_CONTROL	3
#define HD_DISPLAY_ON 		0x04
#define HD_CURSOR_ON		0x02
#define HD_BLINK_ON			0x01
#define HD_CURSOR_MOVE_LEFT 0x10
#define HD_CURSOR_MOVE_RIGHT 0x14
#define HD_DISPLAY_SHIFT_LEFT 0x18
#define HD_DISPLAY_SHIFT_RIGHT 0x1C

#define LCD_WIDTH 20
#define LCD_HEIGHT 4
#define LCD_TOTAL_CHARS (LCD_WIDTH * LCD_HEIGHT)
#define LCD_DDRAM_WRITE 0x80
#define LCD_CGRAM_WRITE 0x40

#define EN_PIN  	BIT2
#define RS_PIN		BIT3
#define RW_PIN		BIT6
#define LCD_PORT_DATA	GPIO_PORTF_DATA_R
#define LCD_PORT_CNTRL	GPIO_PORTD_DATA_R


//TODO: Poll Busy Flag

//private function prototypes

uint8_t lcd_reverseNibble(uint8_t x)
{
	return(((x & 0b0001) << 3) | ((x & 0b0010) << 1) | ((x & 0b0100) >> 1) | ((x & 0b1000) >> 3));
}


void lcd_init(void)
{
	//TODO: Remove waitMillis after commands -- poll busy flag in sendCommand
	volatile uint32_t i = 0;
	SYSCTL_RCGCGPIO_R |= BIT3 | BIT5; //Turn on PORTD, PORTF sys clock

	//Set port to output
	GPIO_PORTF_DIR_R |= 0x1E; //Pins 1:4
	GPIO_PORTF_DEN_R |= 0x1E;

	GPIO_PORTD_DIR_R |= (EN_PIN | RS_PIN | RW_PIN);
	GPIO_PORTD_DEN_R |= (EN_PIN | RS_PIN | RW_PIN);

	LCD_PORT_CNTRL &= ~(EN_PIN | RW_PIN | RS_PIN);

	//Delay 40msec after power applied
	timer_waitMillis(50);

	//Wake up
	lcd_sendNibble(0x03);
	timer_waitMillis(10);

	lcd_sendNibble(0x03);
	timer_waitMicros(170);

	lcd_sendNibble(0x03);
	timer_waitMicros(170);

	lcd_sendNibble(0x02);			//Function set 4 bit
	timer_waitMillis(1);

	lcd_sendCommand(0x28);			//Function 4 bit / 2 lines
	timer_waitMillis(1);

	//lcd_sendCommand(0x10);			//Set cursor
	//timer_waitMillis(1);

	//lcd_sendCommand(HD_BLINK_ON | HD_CURSOR_ON | HD_DISPLAY_ON);
	lcd_sendCommand(0x0F);
	timer_waitMillis(1);

	lcd_sendCommand(0x28);			//Function 4 bit / 2 lines
	timer_waitMillis(1);


	lcd_sendCommand(0x06);			//Increment Cursor / No Display Shift
	timer_waitMillis(1);


	lcd_sendCommand(0x01);			//Return Home
	timer_waitMillis(1);

	lcd_clear();
	timer_waitMillis(1);

}

///Send Char to LCD
void lcd_putc(char data)
{
	//Select - Send Data
	LCD_PORT_CNTRL |= RS_PIN;
	LCD_PORT_CNTRL &= ~(RW_PIN);

	//Send High nibble
	lcd_sendNibble(data >> 4);

	timer_waitMicros(43);

	//Send Lower Nibble
	lcd_sendNibble(data & 0x0F);

	//TODO: Poll Busy flag
}

///Send Character array to LCD
void lcd_puts(char data[])
{
	//While not equal to null
	while(*data != '\0')
	{
		lcd_putc(*data);
		data++;
	}
}

///Send Command to LCD - Position, Clear, Etc.
void lcd_sendCommand(uint8_t data)
{
	//Enable High
	LCD_PORT_CNTRL |= EN_PIN;
	LCD_PORT_CNTRL &= ~(RW_PIN | RS_PIN); // Write Command

	//Send High nibble
	lcd_sendNibble(data >> 4);

	timer_waitMicros(1);
	//Send Lower Nibble
	lcd_sendNibble(data & 0x0F);

	//TODO: Poll Busy Flag
	timer_waitMillis(1);
}


///Send 4bit nibble to lcd, then clear port.
void lcd_sendNibble(uint8_t theNibble)
{
	#ifdef IS_STEPPER_BOARD
	theNibble = lcd_reverseNibble(theNibble);
    #endif
	LCD_PORT_CNTRL |= EN_PIN;
	LCD_PORT_DATA |= (theNibble & 0x0F) << 1; //PORTD1:4

	//Data Hold time before Clock = 40ns -- Change if faster clock
	timer_waitMicros(20);
	//Clock in Data
	LCD_PORT_CNTRL &= ~(EN_PIN);

	timer_waitMicros(20);
	//Clear Port
	LCD_PORT_DATA &= ~((0x0F) << 1);
}

///Clear LCD Screen
void inline lcd_clear(void)
{
	lcd_sendCommand(HD_LCD_CLEAR);

	//This command takes over 1ms to complete
	timer_waitMillis(2);

}

///Return Cursor to 0,0
void inline lcd_home(void)
{
	lcd_sendCommand(HD_RETURN_HOME);
}

///Goto 0 indexed line number
void lcd_gotoLine(uint8_t lineNum)
{

	//Address of the four line elements
	static const uint8_t lineAddress[] = {0x00, 0x40, 0x14, 0x54};

	lineNum = (0x03 & (lineNum-1)); // Mask input for 0 - 3
	lcd_sendCommand(LCD_DDRAM_WRITE | lineAddress[lineNum]);

}

///Set cursor position - top left is 0,0
void lcd_setCursorPos(uint8_t x, uint8_t y) {
	static const uint8_t lineAddresses[] = {0x00, 0x40, 0x14, 0x54};

	if(x >= 20 || y >= 4) {
		//Invalid coordinates
		return;
	}

	//Compute the location index
	uint8_t index = lineAddresses[y] + x;

	//Set the cursor index
	lcd_sendCommand(0x80 | index);
}

/// Print a formatted string to the LCD screen
/**
 * Mimics the C library function printf for writing to the LCD screen.  The function is buffered; i.e. if you call
 * lprintf twice with the same string, it will only update the LCD the first time.
 *
 * Google "printf" for documentation on the formatter string.
 *
 * Code from this site was also used: http://www.ozzu.com/cpp-tutorials/tutorial-writing-custom-printf-wrapper-function-t89166.html
 * @author Kerrick Staley & Chad Nelson
 * @date 05/16/2012
 */

void lcd_printf(const char *format, ...) {
	static char lastbuffer[LCD_TOTAL_CHARS + 1];

	char buffer[LCD_TOTAL_CHARS + 1];
	va_list arglist;
	va_start(arglist, format);
	vsnprintf(buffer, LCD_TOTAL_CHARS + 1, format, arglist);

	if (!strcmp(lastbuffer, buffer))
		return;

	strcpy(lastbuffer, buffer);
	lcd_clear();
	char *str = buffer;
	int charnum = 0;
	while (*str && charnum < LCD_TOTAL_CHARS) {
		if (*str == '\n') {
			/* fill remainder of line with spaces */
			charnum += LCD_WIDTH - charnum % LCD_WIDTH;
		} else {
			lcd_putc(*str);
			charnum++;
		}

		str++;

		/*
		 * The LCD's lines are not sequential; for future reference, the address are like
		 * 0x00...0x13 : line 1
		 * 0x14...0x27 : line 3
		 * 0x28...0x3F : random junk
		 * 0x40...0x53 : line 2
		 * 0x54...0x68 : line 4
		 *
		 * The cursor position must be reset at the end of every line, otherwise, after writing line 1, it writes line 3 and then nothingness
		 */

		if (charnum % LCD_WIDTH == 0) {
			switch (charnum / LCD_WIDTH) {
			case 1:
				lcd_gotoLine(2);
				break;
			case 2:
				lcd_gotoLine(3);
				break;
			case 3:
				lcd_gotoLine(4);
			}
		}
	}
	va_end(arglist);
}

lcd.h
/*
 * lcd.h
 *
 *  Created on: Mar 1, 2016
 *      Author: nbergman
 */

#ifndef LCD_H_
#define LCD_H_

//#define IS_STEPPER_BOARD
//#define IS_STEPPER_BOARD

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inc/tm4c123gh6pm.h>
#include "Timer.h"

/// Extra function for the stepper motor board
uint8_t lcd_reverseNibble(uint8_t x);

/// Initialize PORTB0:6 to Communicate with LCD
void lcd_init(void);

///Send Char to LCD
void lcd_putc(char data);

///Send Character array to LCD
void lcd_puts(char data[]);

///Clear LCD Screen
void inline lcd_clear(void);

///Return Cursor to 0,0
void inline lcd_home(void);

///Goto Line on LCD - 0 Indexed
void lcd_gotoLine(uint8_t lineNum);

///Set cursor position - top left is 0,0
void lcd_setCursorPos(uint8_t x, uint8_t y);

void lcd_printf(const char *format, ...);

///Send command to LCD - Position, Clear, Etc.
void lcd_sendCommand(uint8_t data);

///Send 4bit nibble to lcd, then clear port
void lcd_sendNibble(uint8_t theNibble);


#endif /* LCD_H_ */

Ping_template.h
/**
 * Driver for ping sensor
 * @file ping.c
 * @author
 */
#ifndef PING_H_
#define PING_H_

#include <stdint.h>
#include <stdbool.h>
#include <inc/tm4c123gh6pm.h>
#include "driverlib/interrupt.h"

/**
 * Initialize ping sensor. Uses PB3 and Timer 3B
 */
void ping_init (void);

/**
 * @brief Trigger the ping sensor
 */
void ping_trigger (void);

/**
 * @brief Timer3B ping ISR
 */
void TIMER3B_Handler(void);

/**
 * @brief Calculate the distance in cm
 *
 * @return Distance in cm
 */
float ping_getDistance (void);

#endif /* PING_H_ */

Timer.c

/*
 * timer.c
 *
 *  Created on: Mar 15, 2019
 *      @author Isaac Rex
 *      Adapted from (and compatible with) Eric Middleton's timer utility
 */

// TODO: Check value of MICROS_PER_TICK

#include "Timer.h"

// 65000 gives a countdown time of exactly 65ms TODO: is it 65000 or 64999?
#define MICROS_PER_TICK 64999UL // Number of microseconds in one timer cycle

/**
 * @brief Tracks if the clock is currently running or stopped
 *
 */
unsigned char _running = 0;

/**
 * @brief Tracks the number of milliseconds passed since a call to startClock()
 *
 */
volatile unsigned int _timeout_ticks;

/**
 * @brief Initialize and start the clock at 0. If the clock is
 * already running on a call, reset the time count back to 0. Uses TIMER5.
 *
 */
void timer_init(void) {
    if (!_running) {
        SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R5; // Turn on clock to TIMER5
        TIMER5_CTL_R &= ~TIMER_CTL_TAEN;           // Disable TIMER5 for setup
        TIMER5_CFG_R = TIMER_CFG_16_BIT;           // Set as 16-bit timer
        TIMER5_TAMR_R = TIMER_TAMR_TAMR_PERIOD;    // Periodic, countdown mode
        TIMER5_TAILR_R = MICROS_PER_TICK - 1;      // Countdown time of 65ms
        TIMER5_ICR_R |= TIMER_ICR_TATOCINT; // Clear timeout interrupt status
        TIMER5_TAPR_R = 0x0F;               // 15 gives a period of 1us
        TIMER5_IMR_R |= TIMER_IMR_TATOIM;   // Allow TIMER5 timeout interrupts
        NVIC_PRI23_R |= NVIC_PRI23_INTA_M;  // Priority 7 (lowest)
        NVIC_EN2_R |= (1 << 28);             // Enable TIMER5 interrupts

        IntRegister(INT_TIMER5A, timer_clockTickHandler); // Bind the ISR
        TIMER5_CTL_R |= TIMER_CTL_TAEN; // Start TIMER5 counting

        _running = 1;
    }
}

/**
 * @brief Stop the clock and free up TIMER5. Resets the value returned by
 * timer_getMillis() and timer_getMicros().
 *
 */
void timer_stop(void) {
    TIMER5_CTL_R &= ~TIMER_CTL_TAEN;            // Disable TIMER5
    _timeout_ticks = 0;                         // Reset tick counter
    TIMER5_TAV_R = MICROS_PER_TICK;             // Set TIMER5 back to the top
    SYSCTL_RCGCTIMER_R &= ~SYSCTL_RCGCTIMER_R5; // Turn off clock to TIMER5
    _running = 0;
}

/**
 * @brief Pauses the clock at the current value.
 *
 */
void timer_pause(void) {
    TIMER5_CTL_R &= ~TIMER_CTL_TAEN; // Disable TIMER5
    _running = 0;
}

/**
 * @brief Resumes the clock after a call to pauseClock().
 *
 */
void timer_resume(void) {
    TIMER5_CTL_R |= TIMER_CTL_TAEN; // Enable TIMER5
    _running = 1;
}

/**
 * @brief Returns the number milliseconds that have passed since startClock()
 * was called. Value rolls over after about 49 days.
 *
 * @return unsigned int number of milliseconds since a call to
 * timer_startClock()
 */
unsigned int timer_getMillis(void) {
    unsigned int ticks;
    unsigned int millis;

    TIMER5_IMR_R &= ~TIMER_IMR_TATOIM; // Disable timeout interrupts

    millis = (MICROS_PER_TICK - TIMER5_TAR_R & 0xFFFF) / 1000;
    if (TIMER5_RIS_R & TIMER_RIS_TATORIS) {
        // If the timer overflows while we're getting the time
        ticks = (_timeout_ticks + 1);
        millis = 0;
    } else {
        ticks = _timeout_ticks;
    }

    TIMER5_IMR_R |= TIMER_IMR_TATOIM; // Reenable interrupts from TIMER timeout

    return ticks * (MICROS_PER_TICK / 1000) + millis;
}

/**
 * @brief Returns the number of microseconds passed since a call to
 * startClock(). Value rolls over after about 71 minutes.
 *
 * @return unsigned int number of microseconds since a call to startClock()
 */
unsigned int timer_getMicros(void) {
    unsigned int ticks;
    unsigned int micros;
    if(!_running){
           timer_init();
    }
    TIMER5_IMR_R &= ~TIMER_IMR_TATOIM; // Disable TIMER5 timeout interrupts

    micros = MICROS_PER_TICK - TIMER5_TAR_R & 0xFFFF;

    if (TIMER5_RIS_R & TIMER_RIS_TATORIS) {
        // If the timer overflows while we're getting the time
        ticks = (_timeout_ticks + 1);
        micros = 0;
    } else {
        ticks = _timeout_ticks;
    }

    TIMER5_IMR_R |= TIMER_IMR_TATOIM; // Reenable TIMER5 interrupts

    return ticks * MICROS_PER_TICK + micros;
}

/**
 * @brief Pauses execution for the specified number of microseconds.
 *
 * @param delay_time number of microseconds to pause for
 */
//unsigned int
void timer_waitMicros(uint32_t delay_time) {

    if (delay_time <= 2) {
        // Overhead of the function call is around 1.5us
        return;
    } else {
        delay_time -= 2;
    }

    while (delay_time > 0) { // ldr: 2, cmp: 1, bne: 1; 4 cycles
        // 16 cycles = 1us: need 16 - 9 = 7 NOP cycles
        // Experimentally, 6 is accurate. Missing a cycle?
        asm(" NOP"
            "\n"
            " NOP"
            "\n"
            " NOP"
            "\n"
            " NOP"
            "\n"
            " NOP"
            "\n"
            " NOP");
        delay_time--; // ldr: 2, subs: 1, str: 2; 5 cycles
    }
}

/**
 * @brief Pauses execution for the specified number of milliseconds.
 *
 * @param delay_time number of milliseconds to pause for
 */
//unsigned int
void timer_waitMillis(uint32_t delay_time) {

    unsigned int start = timer_getMicros();
    unsigned int current_micros = timer_getMicros();

    while (delay_time > 0) {
        current_micros = timer_getMicros();
        // Uses a while loop (instead of if) in case a long ISR is called
        while (delay_time > 0 && ((current_micros - start) >= 1000)) {
            delay_time--;
            start += 1000;
            current_micros = timer_getMicros();
        }
    }
}

/**
 * @brief ISR handler to increment the timeout variable for tracking total
 * milliseconds
 *
 */
static void timer_clockTickHandler() {
    TIMER5_ICR_R |= TIMER_ICR_TATOCINT; // Clear interrupt flag
    _timeout_ticks++;
}


Timer.h
/*
 * timer.h
 *
 *  Created on: Mar 15, 2019
 *      @author Isaac Rex
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <inc/tm4c123gh6pm.h>
#include <stdbool.h>
#include <stdint.h>
#include "driverlib/interrupt.h"

/**
 * @brief Initialize and start the clock at 0. If the clock is
 * already running on a call, reset the time count back to 0. Uses TIMER5.
 *
 */
void timer_init(void);

/**
 * @brief Stop the clock and free up TIMER5. Resets the value returned by
 * getMillis() and getMicros().
 *
 */
void timer_stop(void);

/**
 * @brief Pauses the clock at the current value.
 *
 */
void timer_pause(void);

/**
 * @brief Resumes the clock after a call to pauseClock().
 *
 */
void timer_resume(void);

/**
 * @brief Returns the number milliseconds that have passed since startClock()
 * was called. Value rolls over after about 49 days.
 *
 * @return unsigned int number of milliseconds since a call to
 * timer_startClock()
 */
unsigned int timer_getMillis(void);

/**
 * @brief Returns the number of microseconds passed since a call to
 * startClock(). Value rolls over after about 71 minutes.
 *
 * @return unsigned int number of microseconds since a call to startClock()
 */
unsigned int timer_getMicros(void);

/**
 * @brief Pauses execution for the specified number of milliseconds.
 *
 * @param delay_time number of milliseconds to pause for
 */
void timer_waitMillis(unsigned int delay_time);

/**
 * @brief Pauses execution for the specified number of microseconds.
 *
 * @param delay_time number of microseconds to pause for
 */
void timer_waitMicros(unsigned int delay_time);

// TODO: Implement
/**
 * @brief Sets up an interrupt to call the given function once every given
 * milliseconds. Uses TIMER4 for the countdown. Function f executes inside an
 * ISR, so keep the passed function as short as possible. Maximum interval time
 * is TODO: calculate
 *
 * @param f the function to call
 * @param millis the interval between calls
 */
void timer_fireEvery(void (*f)(void), int millis);

// TODO: Implement
/**
 * @brief Sets up an interrupt to call the given function after the given number
 * of milliseconds. Uses TIMER4 for the countdown, and thus can only be used
 * when timer_fireEvery() and timer_fireFor() are not being used. Function f
 * executes inside an ISR and should be kept as short as possible.
 *
 * @param f the function to call
 * @param millis milliseconds until call
 */
void timer_fireOnce(void (*f)(void), int millis);

// TODO: Implement
/**
 * @brief Sets up an interrupt to call the given function after the given number
 * of milliseconds for the given number of times. Uses TIMER4 for the countdown,
 * and thus can only be used when fireOnce() and fireEvery() are not being used.
 * Function f executes inside an ISR and should be kept as short as possible.
 * Maximum interval time is TODO: calculate
 *
 * @param f the function to call
 * @param millis milliseconds until call
 * @param times number of times to call f
 */
void timer_fireFor(void (*f)(void), int millis, int times);

/**
 * @brief ISR handler to increment the timeout variable for tracking total
 * milliseconds
 *
 */
static void timer_clockTickHandler();

#endif /* TIMER_H_ */

Lab9_template.c
/**
 * @file lab9_template.c
 * @author
 * Template file for CprE 288 Lab 9
 */

#include "Timer.h"
#include "lcd.h"
#include "ping.h"

// Uncomment or add any include directives that are needed

#warning "Possible unimplemented functions"
#define REPLACEME 0

int main(void) {
	timer_init(); // Must be called before lcd_init(), which uses timer functions
	lcd_init();
	ping_init();

	// YOUR CODE HERE

	while(1)
	{

      // YOUR CODE HERE


	}

}

ping.c

/**
 * Driver for ping sensor
 * @file ping.c
 * @author
 */

#include "ping.h"
#include "Timer.h"

volatile unsigned long START_TIME = 0;
volatile unsigned long END_TIME = 0;
volatile enum{LOW, HIGH, DONE} STATE = LOW; // State of ping echo pulse

void ping_init (void){

  // YOUR CODE HERE

    IntRegister(INT_TIMER3B, TIMER3B_Handler);

    IntMasterEnable();

    // Configure and enable the timer
    TIMER3_CTL_R ???;
}

void ping_trigger (void){
    STATE = LOW;
    // Disable timer and disable timer interrupt
    TIMER3_CTL_R ???;
    TIMER3_IMR_R ???;
    // Disable alternate function (disconnect timer from port pin)
    GPIO_PORTB_AFSEL_R ???;

    // YOUR CODE HERE FOR PING TRIGGER/START PULSE

    // Clear an interrupt that may have been erroneously triggered
    TIMER3_ICR_R ???
    // Re-enable alternate function, timer interrupt, and timer
    GPIO_PORTB_AFSEL_R ???;
    TIMER3_IMR_R ???;
    TIMER3_CTL_R ???;
}

void TIMER3B_Handler(void){

  // YOUR CODE HERE
  // As needed, go back to review your interrupt handler code for the UART lab.
  // What are the first lines of code in the ISR? Regardless of the device, interrupt handling
  // includes checking the source of the interrupt and clearing the interrupt status bit.
  // Checking the source: test the MIS bit in the MIS register (is the ISR executing
  // because the input capture event happened and interrupts were enabled for that event?
  // Clearing the interrupt: set the ICR bit (so that same event doesn't trigger another interrupt)
  // The rest of the code in the ISR depends on actions needed when the event happens.

}

float ping_getDistance (void){

    // YOUR CODE HERE

}


Uart-interupt.c
/*
*
*   uart-interrupt.c
*
*
*
*   @author
*   @date
*/

// The "???" placeholders should be the same as in your uart.c file.
// The "?????" placeholders are new in this file and must be replaced.

#include <inc/tm4c123gh6pm.h>
#include <stdint.h>
#include "uart-interrupt.h"

// These variables are declared as examples for your use in the interrupt handler.
volatile char command_byte = -1; // byte value for special character used as a command
volatile int command_flag = 0; // flag to tell the main program a special command was received

void uart_interrupt_init(void){
	//TODO
  //enable clock to GPIO port B
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;

  //enable clock to UART1
  SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R1;

  //wait for GPIOB and UART1 peripherals to be ready
  while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R1) == 0) {};
  while ((SYSCTL_PRUART_R & SYSCTL_PRUART_R1) == 0) {};

  //enable digital functionality on port B pins
  GPIO_PORTB_DEN_R |= (1<<0) | (1<<1);

  //enable alternate functions on port B pins
  GPIO_PORTB_AFSEL_R |= (1<<0) | (1<<1);

  //enable UART1 Rx and Tx on port B pins
  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R & 0xFFFFFF00) | 0x00000011;

  //calculate baud rate
  uint16_t iBRD = 8; //use equations
  uint16_t fBRD = 44; //use equations

  //turn off UART1 while setting it up
  UART1_CTL_R &= ~UART_CTL_UARTEN;

  //set baud rate
  //note: to take effect, there must be a write to LCRH after these assignments
  UART1_IBRD_R = iBRD;
  UART1_FBRD_R = fBRD;

  //set frame, 8 data bits, 1 stop bit, no parity, no FIFO
  //note: this write to LCRH must be after the BRD assignments
  UART1_LCRH_R = (UART_LCRH_WLEN_8 | UART_LCRH_FEN);

  //use system clock as source
  //note from the datasheet UARTCCC register description:
  //field is 0 (system clock) by default on reset
  //Good to be explicit in your code
  UART1_CC_R = 0x0;

  //////Enable interrupts

  //first clear RX interrupt flag (clear by writing 1 to ICR)
  UART1_ICR_R |= 0b00010000;

  //enable RX raw interrupts in interrupt mask register
  UART1_IM_R |= (1 << 4);

  //NVIC setup: set priority of UART1 interrupt to 1 in bits 21-23
  NVIC_PRI1_R = (NVIC_PRI1_R & 0xFF0FFFFF) | 0x00200000;

  //NVIC setup: enable interrupt for UART1, IRQ #6, set bit 6
  NVIC_EN0_R |= (1 << 5);

  //tell CPU to use ISR handler for UART1 (see interrupt.h file)
  //from system header file: #define INT_UART1 22
  IntRegister(INT_UART1, UART1_Handler);

  //globally allow CPU to service interrupts (see interrupt.h file)
  IntMasterEnable();

  //re-enable UART1 and also enable RX, TX (three bits)
  //note from the datasheet UARTCTL register description:
  //RX and TX are enabled by default on reset
  //Good to be explicit in your code
  //Be careful to not clear RX and TX enable bits
  //(either preserve if already set or set them)
  UART1_CTL_R |= (UART_CTL_RXE | UART_CTL_TXE | UART_CTL_UARTEN);

}

void uart_sendChar(char data){

    // Wait until there is room in the FIFO
    while((UART1_FR_R & UART_FR_TXFF) != 0);
    // Send data
    UART1_DR_R = data;
}

char uart_receive(void){
    while((UART1_FR_R & UART_FR_RXFE) != 0){}
      // Return the data
      return (char)(UART1_DR_R & 0xFF); //(char)(UART1_DR_R & 0xFF);
}

void uart_sendStr(const char *str){
    int i = 0;
        while(str[i]){
            uart_sendChar(str[i++]);
        }
        uart_sendChar('\n');
        uart_sendChar('\r');
}

// Interrupt handler for receive interrupts
void UART1_Handler(void)
{
    char byte_received;
    //check if handler called due to RX event
    if (UART1_MIS_R & UART_MIS_RXMIS)
    {
        //byte was received in the UART data register
        //clear the RX trigger flag (clear by writing 1 to ICR)
        UART1_ICR_R |= 0b00010000;

        //read the byte received from UART1_DR_R and echo it back to PuTTY
        //ignore the error bits in UART1_DR_R
        byte_received = UART1_DR_R & 0xFF;
        uart_sendChar(byte_received);

        //if byte received is a carriage return
        if (byte_received == '\r')
        {
            //send a newline character back to PuTTY
            uart_sendChar('\n');
        }
        else
        {
            //AS NEEDED
            //code to handle any other special characters
            //code to update global shared variables
            //DO NOT PUT TIME-CONSUMING CODE IN AN ISR

            if (byte_received == command_byte)
            {
              command_flag = 1;
            }
        }
    }
}


uart-interupt.h
/*
*
*   uart-interrupt.h
*
*   Used to set up the RS232 connector and WIFI module
*   Uses RX interrupt
*   Functions for communicating between CyBot and PC via UART1
*   Serial parameters: Baud = 115200, 8 data bits, 1 stop bit,
*   no parity, no flow control on COM1, FIFOs disabled on UART1
*
*   @author Dane Larson
*   @date 07/18/2016
*   Phillip Jones updated 9/2019, removed WiFi.h, Timer.h
*   Diane Rover updated 2/2020, added interrupt code
*/

#ifndef UART_H_
#define UART_H_

#include <inc/tm4c123gh6pm.h>
#include <stdint.h>
#include <stdbool.h>
#include "driverlib/interrupt.h"

// Notice that interrupt.h provides library function prototypes for IntMasterEnable() and IntRegister()

// The following externals are global variables defined in uart-interrupt.c for use with the interrupt handler.
// Using extern here, the global variables become visible to other c files that include uart-interrupt.h
// Extern does not allocate storage for a variable. It tells the compiler that the variable is defined in another file.
//extern volatile char receive_buffer[]; // buffer for characters received from PuTTY
//extern volatile int receive_index; // index to keep track of characters in buffer
extern volatile char command_byte; // byte value for special character used as a command
extern volatile int command_flag; // flag to tell the main program a special command was received

// UART1 device initialization for CyBot to PuTTY
void uart_interrupt_init(void);

// Send a byte over UART1 from CyBot to PuTTY
void uart_sendChar(char data);

// CyBot waits (i.e. blocks) to receive a byte from PuTTY
// returns byte that was received by UART1
// Not used with interrupts; see UART1_Handler
char uart_receive(void);

// Send a string over UART1
// Sends each char in the string one at a time
void uart_sendStr(const char *data);

// Interrupt handler for receive interrupts
void UART1_Handler(void);

#endif /* UART_H_ */

map.h
#include "open_interface.h"
#include "Timer.h"
#include "lcd.h"
#include "cyBot_Scan.h"
#include "uart-interrupt.h"

typedef struct {
    float distance;
    int angleStart;
    int angleEnd;
    int angleMiddle;
    float size;
    int isObject; // when a scan detects a object it increases this count (only stored in first object of the list)
} Obj;

cyBOT_Scan_t scan;
oi_t*sensor_data;

void calibrate();

void scanSonar(cyBOT_Scan_t scan, oi_t*sensor_data);

void scanPointAt(int angle);

uint32_t IR_Scan();

void print(char str[]);

void lab07(void);

void initMap();

void initADC();

void lab08();

void scanIRcal(cyBOT_Scan_t scan);

uint32_t IR_Read();

void wait_sec(int sec);

uint32_t read_ADC();

Map.c
#include "open_interface.h"
#include "movement.h"
#include "Timer.h"
#include "lcd.h"
#include "cyBot_Scan.h"
#include "uart-interrupt.h"
#include "Map.h"
#include <math.h>


void calibrate(){
    int i = 0;
    char temp[100];

    timer_waitMillis(1000);
    scanPointAt(90);
    timer_waitMillis(1000);

    for(i=0; i<50; i++){
        sprintf(temp, "%d",read_ADC());
        uart_sendStr(temp);
    }

}






void initMap(){
    timer_init();
    //oi_t*sensor_data = oi_alloc();
    //oi_init(sensor_data);
    lcd_init();
    uart_interrupt_init();
    cyBOT_init_Scan(0b0111);
    uart_sendStr("READY");
}






void initADC(){

    // Enable the clock to ADC0 and the port that contains the analog input
    SYSCTL_RCGCADC_R |= 0x01; // Enable clock to ADC0
    SYSCTL_RCGCGPIO_R |= 0x02; // Enable clock to PORTE (for PE3/AIN0)

    // Allow time for the clock to start
    volatile unsigned long delay = SYSCTL_RCGCGPIO_R;

    // Configure PE3 for ADC input
    GPIO_PORTB_DIR_R &= ~0x10; // Make PE3 input
    GPIO_PORTB_AFSEL_R |= 0x10; // Enable alternate function
    GPIO_PORTB_DEN_R &= ~0x10; // Disable digital function
    GPIO_PORTB_AMSEL_R |= 0x10; // Enable analog function

    // Disable SS3 during configuration
    ADC0_ACTSS_R &= ~0x08;

    // Configure SS3 settings
    ADC0_EMUX_R &= ~0xF000; // Software trigger conversion
    ADC0_SSMUX3_R = 10; // Select AIN10 as input
    ADC0_SSCTL3_R = 0x06; // Take one sample at a time, enable interrupt, and signal end of sequence

    // Enable the sample sequencer 3
    ADC0_ACTSS_R |= 0x08;

    // Optional: Configure interrupt system for ADC0 SS3 if needed

}






uint32_t read_ADC(){
    ADC0_PSSI_R = 8;
    while((ADC0_RIS_R & 8) == 0){}
    ADC0_ISC_R = 8;
    return ADC0_SSFIFO3_R;

}





void lab07(void){
    initMap();
    initADC();
    //oi_free(sensor_data);
    scanIRcal(scan);
}







void lab08(void){
    initMap();
    initADC();

    scanPointAt(90);
    timer_waitMillis(1000);

    //calibrate();

    char temp[100];
    uint32_t final = IR_Read();

    sprintf(temp, "Distance Value: %dcm",final);
    uart_sendStr(temp);
    uart_sendStr("Press 's' to read again");
    while(uart_receive() != 's'){}
}




void scanPointAt(int angle){
    cyBOT_Scan(angle, &scan);
}


uint32_t IR_Scan(int angle){
    scanPointAt(angle);
    return IR_Read();
}



uint32_t IR_Read()
{
    double Buffer;
    uint32_t final;
    uint32_t IROut;//the IR value of the scan being sent to the LCD
    uint32_t DisOut;//the Distance value of the scan being sent to the LCD
    int i = 0;
    final = 0;//resets the value of final

    //while(uart_receive() != 's'){}//waits to start a read until the key 's' is pressed

    for(i = 0; i < 16; i++){ final += read_ADC();}//the sum of 16 concurrent reads if the ADC
    final = final/16;//the average of 16 concurrent reads if the ADC
    IROut = final;
    Buffer = pow(final,-1.001);
    final = (uint32_t)(34914*Buffer)-10;//apply the curve fitting formula
    DisOut = final;
    //curve fitting formula is 34914*(IR_VALUE)^-1.001
    lcd_printf("IR Value: %d \nDistance Value: %d", IROut, DisOut);
    return final;
}




void scanIRcal(cyBOT_Scan_t scan){
    oi_t*sensor_data = oi_alloc();
        oi_init(sensor_data);
        //move_forward(sensor_data, 0);
    int i=0;
    char temp[50];

    sprintf(temp,"Press s");
    uart_sendStr(temp);

    while(uart_receive() != 's'){} // wait for
    cyBOT_Scan(0, &scan);
    uint32_t last;
    uint32_t dist = IR_Scan(0);

    Obj obj[5];
    int buffer = 10;
    int currObj = 0;
    int scanning = 0;

    //float changeL[91];

        for(i=2;i<=180;i+=2){ // Start Scan
            cyBOT_Scan(i, &scan);
            last = dist;
            dist = IR_Scan(i);
            //changeL[(i/2)-1] = last-dist;
            sprintf(temp,"Angle:%2i \t Distance:%5.3d \t Change:%3.1d", i,dist, last-dist);
            uart_sendStr(temp);


            // Object Build
                    //scan start
                    if((int)(last-dist) > buffer && scanning == 0){ // if change is less than -250 and we arnt scanning
                        uart_sendStr("Start");
                        scanning =1; // start scanning
                        obj[currObj].angleStart = i;
                        obj[currObj].distance = scan.sound_dist;
                    // scan still
                    } else if(scanning){// if still scanning not first scan
                        obj[currObj].distance += scan.sound_dist; // to take average later
                    }
                    //scan end
                    if((int)(last-dist) < -1*buffer && scanning == 1){// if scanning, and buffer is greater
                        uart_sendStr("End");
                        obj[currObj].angleEnd = i;
                        obj[currObj].angleMiddle = (obj[currObj].angleStart+obj[currObj].angleEnd)/2;
                        obj[currObj].distance /= (obj[currObj].angleEnd/2 - obj[currObj].angleStart/2)+1;// average distance
                        obj[currObj].size = 228;
                        obj[currObj].size = (2*3*obj[currObj].distance);// Thats a lot of words... To bad I aint reading them
                        obj[currObj].size *= ((obj[currObj].angleEnd - obj[currObj].angleStart)/360.0);
                        //(tan(obj[currObj].angleEnd-obj[currObj].angleStart) * obj[currObj].distance) * -1

                        if((obj[currObj].angleEnd - obj[currObj].angleStart) > 2){
                            obj[currObj].isObject = 1;
                        } else{
                            obj[currObj].isObject = 0;
                        }

                        currObj++;
                        scanning = 0;
                    }
              // End Object Build


            }

    //oi_free(sensor_data);

    Obj smallest;
    smallest.size = 100; // just a large number

    for(i=0 ; i<10;i++){
        if(obj[i].isObject == 1){
            //cyBOT_Scan(obj[i].angleMiddle, &scan);
            if(obj[i].size <= smallest.size){
                smallest = obj[i];
            }
        }
        //uart_sendStr("NOT VALID");
    }

    sprintf(temp,"Angle:%2i", smallest.angleMiddle);
    uart_sendStr(temp);

    cyBOT_Scan(smallest.angleMiddle, &scan);

    if(smallest.angleMiddle > 90){
        turn_left(sensor_data, (90-smallest.angleMiddle));
    }else{
        turn_right(sensor_data,  (smallest.angleMiddle)-90);
    }

    //while(uart_receive() != 's'){} // wait for
    move_forward(sensor_data, smallest.distance*5.0);




}










/*
void scanIRcal(cyBOT_Scan_t scan){
    oi_t*sensor_data = oi_alloc();
        oi_init(sensor_data);
        move_forward(sensor_data, 0);
    int i=0;
    char temp[50];

    sprintf(temp,"Press s");
    uart_sendStr(temp);

    while(uart_receive() != 's'){} // wait for
    cyBOT_Scan(0, &scan);
    float last;
    float dist = scan.IR_raw_val;

    Obj obj[5];
    int buffer = 80;
    int currObj = 0;
    int scanning = 0;

    //float changeL[91];

        for(i=2;i<=180;i+=2){ // Start Scan
            cyBOT_Scan(i, &scan);
            last = dist;
            dist = scan.IR_raw_val;
            //changeL[(i/2)-1] = last-dist;
            sprintf(temp,"Angle:%2i \t Distance:%5.3lf \t Change:%3.1lf", i,dist, last-dist);
            uart_sendStr(temp);


            // Object Build
                    //scan start
                    if(last-dist< -1*buffer && scanning == 0){ // if change is less than -250 and we arnt scanning
                        uart_sendStr("Start");
                        scanning =1; // start scanning
                        obj[currObj].angleStart = i;
                        obj[currObj].distance = scan.sound_dist;
                    // scan still
                    } else if(scanning){// if still scanning not first scan
                        obj[currObj].distance += scan.sound_dist; // to take average later
                    }
                    //scan end
                    if(last-dist > buffer && scanning == 1){// if scanning, and buffer is greater
                        uart_sendStr("End");
                        obj[currObj].angleEnd = i;
                        obj[currObj].angleMiddle = (obj[currObj].angleStart+obj[currObj].angleEnd)/2;
                        obj[currObj].distance /= (obj[currObj].angleEnd/2 - obj[currObj].angleStart/2)+1;// average distance
                        obj[currObj].size = 228;
                        obj[currObj].size = (2*3*obj[currObj].distance);// Thats a lot of words... To bad I aint reading them
                        obj[currObj].size *= ((obj[currObj].angleEnd - obj[currObj].angleStart)/360.0);
                        //(tan(obj[currObj].angleEnd-obj[currObj].angleStart) * obj[currObj].distance) * -1

                        if((obj[currObj].angleEnd - obj[currObj].angleStart) > 2){
                            obj[currObj].isObject = 1;
                        } else{
                            obj[currObj].isObject = 0;
                        }

                        currObj++;
                        scanning = 0;
                    }
              // End Object Build


            }

    //oi_free(sensor_data);

    Obj smallest;
    smallest.size = 100; // just a large number

    for(i=0 ; i<10;i++){
        if(obj[i].isObject == 1){
            //cyBOT_Scan(obj[i].angleMiddle, &scan);
            if(obj[i].size <= smallest.size){
                smallest = obj[i];
            }
        }
        //uart_sendStr("NOT VALID");
    }

    cyBOT_Scan(smallest.angleMiddle, &scan);
    if(smallest.angleMiddle > 90){
        turn_left(sensor_data, (smallest.angleMiddle)-90);
    }else{
        turn_right(sensor_data,  90-(smallest.angleMiddle));
    }

    //while(uart_receive() != 's'){} // wait for
    move_forward(sensor_data, smallest.distance*5.0);




}
*/



Movement.c
#include "open_interface.h"
#include "movement.h"
#include "Timer.h"
#include "lcd.h"

double move_forward(oi_t*sensor_data, double distance_mm){
  oi_setWheels(0,0);
  double sum = 0;
  oi_setWheels(100,100); // set... fast
  while(sum < distance_mm){//go untill distance
      oi_setWheels(100,100);
      if(timer_getMillis() > 10000){
          break;
      }
      if( sensor_data -> bumpLeft){
          oi_update(sensor_data);
          move_backward(sensor_data, 150);
          turn_right(sensor_data, 90);
          move_forward(sensor_data, 250);
          turn_left(sensor_data, 90);
      }

      if(sensor_data -> bumpRight){
          oi_update(sensor_data);
               move_backward(sensor_data, 150);
               turn_left(sensor_data, 90);
                move_forward(sensor_data, 250);
                turn_right(sensor_data, 90);
            }
      oi_update(sensor_data);
      sum += sensor_data -> distance; // this is pointer
      lcd_printf("Forward %lf", sum); // this is pointless

  }
  oi_setWheels(0,0); // STOP!
  return(sum); // just becuase double ya know
}

double move_backward(oi_t*sensor_data, double distance_mm){
  oi_setWheels(0,0);
  double sum = 0;
  oi_setWheels(-100,-100); // set... fast
  while(sum > -1*distance_mm){//go untill distance
      oi_update(sensor_data);
      sum += sensor_data -> distance; // this is pointer
      lcd_printf("Back %lf",sum); // this is pointless
  }
  oi_setWheels(0,0); // STOP!
  return(sum); // just becuase double ya know
}

double turn_right(oi_t*sensor_data, double angle_deg){
  oi_setWheels(0,0);
  double sum = 0;
  oi_setWheels(100,-100); // set... fast
  while(sum < angle_deg-7){//go untill distance
      oi_update(sensor_data);
      sum += sensor_data -> angle; // this is pointer
      lcd_printf("Right %lf",sum); // this is pointless
  }
  oi_setWheels(0,0); // STOP!
  return(sum); // just becuase double ya know
}

double turn_left(oi_t*sensor_data, double angle_deg){
  oi_setWheels(0,0);
  double sum = 0;
  oi_setWheels(-100,100); // set... fast
  while(sum < angle_deg-7){//go untill distance
      oi_update(sensor_data);
      sum -= sensor_data -> angle; // this is pointer
      lcd_printf("Left %lf",sum); // this is pointless
  }
  oi_setWheels(0,0); // STOP!
  return(sum); // just becuase double ya know
}


movement.h
#ifndef HEADER_FILE
#define HEADER_FILE
//Function headers and macro definitions

double move_forward(oi_t*sensor_data, double distance_mm);
double move_backward(oi_t*sensor_data, double distance_mm);
double turn_right(oi_t*sensor_data, double angle_deg);
double turn_left(oi_t*sensor_data, double angle_deg);

#endif



