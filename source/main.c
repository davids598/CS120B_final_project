/*	Author: David Strathman
 *  Partner(s) Name:
 *	Lab Section:
 *	Assignment: Final Project, Check-in #1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *  Link to Vid:
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include "../header/timer.h"
#include "../header/io.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
//-------------------------Setup and helper functions---------------------------
typedef struct task {
  signed char state;
  unsigned long int period;
  unsigned long int elapsedTime;
  int (*TickFct)(int);
} task;

void ADC_init() {
  ADMUX|=(0<<REFS0);
  //ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
  ADCSRA |= (1 << ADEN) | (1 << ADSC);
}

short ADC_Switch_Ch(char ch) {
  ADMUX  &= 0xF0;
	ADMUX  |= ch;
  ADCSRA |= (1 << ADSC);
  while((ADCSRA & 0x40) == 0x40);

  return ADC;
}
/*
short ADC_read(short ch)                                 //ADC read
{
	ch &= 0x07;
	ADCSRB &= ~(1<<MUX5);  //use mux 0-7
	ADMUX  &= 0xF8;                                                //clear old analog channel
	ADMUX  |= ch;                                                  //set new analog channel
	ADCSRA |= (1<<ADSC);                                          //Start Single conversion
	while(ADCSRA & (1<<ADSC));                                 //Wait for conversion to complete

	return (ADC);                                               //10 bit ADC result
}
*/
//----------------------------------Globals-------------------------------------
enum Joystick1_positions {J1_UP, J1_DOWN, J1_LEFT, J1_RIGHT, J1_NEUTRAL} Joystick1_position; //Runner
enum Joystick2_positions {J2_UP, J2_DOWN, J2_LEFT, J2_RIGHT, J2_NEUTRAL} Joystick2_position; //Opponent
//----------------------------------Task 1---------------------------------------
enum joystick_checks { joystick_check };

int joystick_Reading(int state) {
  unsigned short runner_x;
  unsigned short opponent_x;
  unsigned short opponent_y;
  switch (state) {
    case joystick_check: state = joystick_check; break;
    default: state = joystick_check; break;
  }

  switch (state) {
    case joystick_check:
      runner_x = ADC_Switch_Ch(0x00);
      opponent_x = ADC_Switch_Ch(0x02);
      opponent_y = ADC_Switch_Ch(0x03);
      //Determine Joystick 1 position
      if (runner_x <= 300) {
        Joystick1_position = J1_LEFT;
      } else if (runner_x >= 725) {
        Joystick1_position = J1_RIGHT;
      } else {
        Joystick1_position = J1_NEUTRAL;
      }
      //Determine Joystick 2 Position
      if (opponent_y >= 800 && (opponent_x >= 300 && opponent_x <= 800)) {
        Joystick2_position = J2_UP;
      } else if (opponent_x <= 100 && opponent_y >= 300) {
        Joystick2_position = J2_LEFT;
      } else if (opponent_x >= 800 && opponent_y >= 300) {
        Joystick2_position = J2_RIGHT;
      } else {
        Joystick2_position = J2_NEUTRAL;
      }
  }
  return state;
}
//------------------------------Task 2------------------------------------------
enum Output_states {output_loop};

int Output(int state) {
  switch (state) {
    case output_loop: state = output_loop; break;
      default:  state = output_loop; break;
  }

  switch (state) {
    case output_loop:
      //Joystick 1 port C Outputs
      PORTC = PORTC & 0x3F;
      if (Joystick1_position == J1_LEFT) {
        PORTC = PORTC | 0x80;
        LCD_Cursor(1);
      } else if (Joystick1_position == J1_RIGHT) {
        PORTC = PORTC | 0x40;
        LCD_Cursor(17);
      }
      //Joystick 2 outputs
      PORTC = PORTC & 0xC7;
      if (Joystick2_position == J2_LEFT) {
        PORTC = PORTC | 0x20;
      } else if (Joystick2_position == J2_UP) {
        PORTC = PORTC | 0x10;
      } else if (Joystick2_position == J2_RIGHT) {
        PORTC = PORTC | 0x08;
      }
  }
  return state;
}
//---------------------------Main and GCD---------------------------------------
unsigned long int findGCD(unsigned long int a, unsigned long int b) {
  unsigned long int c;
  while (1) {
    c = a%b;
    if (c==0) {return b;}
    a = b;
    b = c;
  }
  return 0;
}

int main(void) {
  DDRA = 0x00; PORTA = 0xFF; //Input
  DDRB = 0xFF; PORTB = 0x00; //Output
  DDRC = 0xFF; PORTC = 0x00; //Output
  DDRD = 0xFF; PORTD = 0x00; //Output

  static task task1, task2;
  task *tasks[] = { &task1, &task2 };
  const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

  const char start = -1;

  //Task 1
  task1.state = start;
  task1.period = 1;
  task1.elapsedTime = task1.period;
  task1.TickFct = &joystick_Reading;
  //Task 2
  task2.state = start;
  task2.period = 10;
  task2.elapsedTime = task2.period;
  task2.TickFct = &Output;

  unsigned long GCD = tasks[0]->period;
  for (int i = 1; i < numTasks; i++) {
    GCD = findGCD(GCD, tasks[i]->period);
  }
  TimerSet(GCD);
  TimerOn();
  ADC_init();
  LCD_init();
  LCD_ClearScreen();
  LCD_Cursor(1);

  unsigned short i;
  while (1) {
    for (i = 0; i < numTasks; i++) {
      if (tasks[i]->elapsedTime == tasks[i]->period) {
        tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
        tasks[i]->elapsedTime = 0;
      }
      tasks[i]->elapsedTime += GCD;
    }
    while (!TimerFlag);
    TimerFlag = 0;
  }

  return 0;
}
