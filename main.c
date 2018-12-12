/*
 * a4.c
 *
 * Created: 12/1/2018 6:46:18 PM
 * Author : weijunsyu

 Displays 2 scrolling messages on each line
 Clicking the up button will pause the scrolling
 Clicking the down button will resume scrolling
 Clicking the left button will decrease the scrolling speed
 Clicking the right button will increase the scrolling speed

 Clicking the select button will toggle between the messages and interactive mode

 In interactive mode the user can use the directional buttons to control an "x"
 "o"'s will scroll across the screen from right to left.

 */

//#include <avr/io.h>
//#include <avr/interrupt.h>

#include <stdlib.h>
#include <string.h>
#include "CSC230.h" //university provided library for AVR (ATMEGA2560 board and add-ons)

#define MSG_1 "This is the message on the first line. Here it goes."
#define MSG_2 "--- buy --- more --- pop --- buy"
#define GM_1 "o         o        o       o      o     o    o   o  o o"
#define GM_2 "     o         o       o       o     o     o   o   o o"
#define MAX_STR_LEN 16

#define  ADC_BTN_RIGHT 0x032
#define  ADC_BTN_UP 0x0C3
#define  ADC_BTN_DOWN 0x17C
#define  ADC_BTN_LEFT 0x22B
#define  ADC_BTN_SELECT 0x316
#define ADC_BTN_NONE 0x3E8

void delayMsg(int delay);
void display(char* msg1, char* msg2);
void getMsg(const char* source, int* start, char** msg);
ISR(TIMER0_OVF_vect);
void timer0_setup();
unsigned short poll_adc();
void gameDisplay(const char* s1, int* g1, char** g1p,
                 const char* s2, int* g2, char** g2p);
void game();
void mov_up();
void mov_down();
void mov_left();
void mov_right();

int global_flag = 0;
int x = 0;
int y = 0;
int msg_delay = 2;


int main() {
	int start1;
	int start2;
	char* msg1_p;
	char* msg2_p;

	global_flag = 1;
	start1 = 0;
	start2 = 0;
	msg1_p = malloc((MAX_STR_LEN + 1) * sizeof(char));
	msg2_p = malloc((MAX_STR_LEN + 1) * sizeof(char));

	msg1_p[MAX_STR_LEN] = '\0';
	msg2_p[MAX_STR_LEN] = '\0';

	timer0_setup();

	//ADC Set up
	ADCSRA = 0x87;
	ADMUX = 0x40;

	lcd_init();
	sei(); /* enable interrupts */

    while (1) {
		if (global_flag == 1) {
			getMsg(MSG_1, &start1, &msg1_p);
			getMsg(MSG_2, &start2, &msg2_p);
			display(msg1_p, msg2_p);
			delayMsg(msg_delay);
		}
		else if (global_flag == 0) {
			_delay_ms(500);
		}
		else if (global_flag == 2) {
			game();
		}
	}

	/* should never be reached but if so free memory */
	free(msg1_p);
	free(msg2_p);
}

void game() {
	int g1;
	int g2;
	char* g1_p;
	char* g2_p;

	g1 = 0;
	g2 = 0;
	g1_p = malloc((MAX_STR_LEN + 1) * sizeof(char));
	g2_p = malloc((MAX_STR_LEN + 1) * sizeof(char));

	g1_p[MAX_STR_LEN] = '\0';
	g2_p[MAX_STR_LEN] = '\0';

	while (global_flag == 2) {
		gameDisplay(GM_1, &g1, &g1_p, GM_2, &g2, &g2_p);
	}

	free(g1_p);
	free(g2_p);
}

void gameDisplay(const char* s1, int* g1, char** g1p,
                 const char* s2, int* g2, char** g2p) {

	getMsg(s1, g1, g1p);
	getMsg(s2, g2, g2p);
	display(*g1p, *g2p);
	_delay_ms(500);
}

/* Get a 16 char message from the source string given some start index.
   Increment index by 1.
   Reset index to 0 if reached end of source string.
   ASSUME PROPER NULL TERMINATING C-STRING. DOES NOT MODIFY NULL CHAR. */
void getMsg(const char* source, int* start, char** msg) {
	int count = 0;
	int wrap = 0;
	while (count < MAX_STR_LEN && source[*start + count] != '\0') {
		count++;
	}
	wrap = MAX_STR_LEN - count;
	memcpy(*msg, &(source[*start]), count);
	(*start)++;

	if (wrap > 0) {
		char* middle = &((*msg)[count]);
		memcpy(middle, source, wrap);
		if (wrap == MAX_STR_LEN) {
			*start = 0;
		}
	}
}

/* Display the messages on lines 1 and 2 of the LCD. */
void display(char* msg1, char* msg2) {

	lcd_blank(32);

	lcd_xy(0,0);
	lcd_puts(msg1);

	lcd_xy(0,1);
	lcd_puts(msg2);
}

ISR(TIMER0_OVF_vect) {
	if (global_flag == 2) {
		lcd_xy(x,y);
		lcd_puts("x");
	}

	unsigned short adc_result = poll_adc();

	if (adc_result > ADC_BTN_NONE) {
		return;
	}
	else if (adc_result < ADC_BTN_RIGHT) {
		if (global_flag == 2) {
			mov_right();
			_delay_ms(50);
		}
		else {
			if (msg_delay < 4) {
				msg_delay++;
				_delay_ms(200);
			}
		}
	}
	else if (adc_result < ADC_BTN_UP) {
		if (global_flag == 1) {
			global_flag = 0;
			_delay_ms(200);
		}
		else if (global_flag == 2) {
			mov_up();
			_delay_ms(50);
		}

	}
	else if (adc_result < ADC_BTN_DOWN) {
		if (global_flag == 0) {
			global_flag = 1;
			_delay_ms(200);
		}
		else if (global_flag == 2) {
			mov_down();
			_delay_ms(50);

		}
	}
	else if (adc_result < ADC_BTN_LEFT) {
		if (global_flag == 2) {
			mov_left();
			_delay_ms(50);
		}
		else {
			if (msg_delay > 0) {
				msg_delay--;
				_delay_ms(200);
			}

		}
	}
	else if (adc_result < ADC_BTN_SELECT) {
		if (global_flag != 2) {
			global_flag = 2;
			_delay_ms(200);
		}
		else {
			global_flag = 1;
			_delay_ms(200);
		}

	}
	else {
		display("ERROR", "UNKNOWN ACTION");
		global_flag = 0;
	}
}

/* From lab09 */
unsigned short poll_adc() {
	unsigned short adc_result = 0; //16 bits

	ADCSRA |= 0x40;
	while((ADCSRA & 0x40) == 0x40); //Busy-wait

	unsigned short result_low = ADCL;
	unsigned short result_high = ADCH;

	adc_result = (result_high<<8)|result_low;
	return adc_result;
}

/* From lab9 */
void timer0_setup(){
	//You can also enable output compare mode or use other
	//timers (as you would do in assembly).

	TIMSK0 = 0x01;
	TCNT0 = 0x00;
	TCCR0A = 0x00;
	TCCR0B = 0x05; //Prescaler of 1024
}

void delayMsg(int delay) {
	if (delay == 0) {
		_delay_ms(2000);
	}
	if (delay == 1) {
		_delay_ms(1000);
	}
	if (delay == 2) {
		_delay_ms(500);
	}
	if (delay == 3) {
		_delay_ms(250);
	}
	if (delay == 4) {
		_delay_ms(125);
	}
}

void mov_up() {
	if (y != 0) {
		y--;
	}
}

void mov_down() {
	if (y != 1) {
		y++;
	}
}

void mov_left() {
	if (x > 0) {
		x--;
	}
}

void mov_right() {
	if (x < 15) {
		x++;
	}
}
