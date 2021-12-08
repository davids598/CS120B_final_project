/*	Author: David Strathman
 *  Partner(s) Name:
 *	Lab Section:
 *	Assignment: Final Project Complete
 *	Exercise Description: [optional - include for your own benefit]
 *
 *  Link to Vid: https://youtu.be/UmNftzKkKAc
 *
 *  Link to Drive: https://drive.google.com/drive/folders/12tOWff0nKDmJeyp0xA47mRrHXJiDrbyQ?usp=sharing
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include "../header/timer.h"
#include "../header/io.h"
#include <avr/eeprom.h>
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
//----------------------------------Globals-------------------------------------
enum Joystick1_positions {J1_UP, J1_DOWN, J1_LEFT, J1_RIGHT, J1_NEUTRAL} Joystick1_position; //Runner
enum Joystick2_positions {J2_UP, J2_DOWN, J2_LEFT, J2_RIGHT, J2_NEUTRAL} Joystick2_position; //Opponent
char top_row[17] = { };
char bottom_row[17] = { };
unsigned char runnerPos = 0;
unsigned short timer = 0;
unsigned char max_time = 0;
unsigned char start = 0;
unsigned char high_score = 0;
unsigned char reset = 0;
unsigned char currC = 0;
unsigned char game_over = 0;
//--------------------------------Custom Characters------------------------------
/*
unsigned char Character1[8] = {
  0x0E,
  0x0A,
  0x0E,
  0x04,
  0x04,
  0x04,
  0x0A,
  0x11
};
unsigned char Character2[8] = { 0x00, 0x0E, 0x1F, 0x15, 0x1B, 0x0E, 0x0A, 0x00 }; // Skull
unsigned char Character3[8] = { 0x11, 0x0E, 0x04, 0x0E, 0x1F, 0x0E, 0x0E, 0x1B }; // Alian
unsigned char Character4[8] = { 0x04, 0x0E, 0x0E, 0x0E, 0x1F, 0x0E, 0x04, 0x04 }; // Sword
unsigned char Character5[8] = { 0x00, 0x0E, 0x15, 0x1F, 0x1F, 0x15, 0x00, 0x00 }; // Ghost
unsigned char Character6[8] = { 0x0A, 0x0A, 0x1F, 0x11, 0x11, 0x0E, 0x04, 0x04 }; // Plug
unsigned char Character7[8] = { 0x00, 0x00, 0x15, 0x15, 0x1F, 0x1F, 0x00, 0x00 }; // Crown
unsigned char Character8[8] = { 0x00, 0x0A, 0x1F, 0x1F, 0x0E, 0x04, 0x00, 0x00 }; // Heart
*/
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
      currC = ~PINC;
      start = (currC & 0x01);
      reset = (currC & 0x02);
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
      } else if (Joystick1_position == J1_RIGHT) {
        PORTC = PORTC | 0x40;
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
//------------------------------Task 3------------------------------------------
enum gameInputs { gameinputloop };

int playerInput(int state) {
  switch (state) {
    case gameinputloop: state = gameinputloop; break;
    default:  state = gameinputloop; break;
  }

  switch (state) {
    case gameinputloop:
      //wiping values at end of rows
      //LCD_Cursor(16);
      //LCD_WriteData("");
      //LCD_Cursor(32);
      //LCD_WriteData("");
      //Read runner joystick input
      if (Joystick1_position == J1_RIGHT) {
        LCD_Cursor(1);
        //LCD_Custom_Char(1, Character1);
        runnerPos = 0;
      }
      else if (Joystick1_position == J1_LEFT) {
        LCD_Cursor(17);
        //LCD_Custom_Char(17, Character1);
        runnerPos = 1;
      } else {
        //No runner input detected, going to previous location
        if (runnerPos == 0) {
          LCD_Cursor(1);
          //LCD_Custom_Char(1, Character1);
        } else {
          LCD_Cursor(17);
          //LCD_Custom_Char(1, Character1);
        }
      }
      break;
  }
  return state;
}
//------------------------------Task 4------------------------------------------
enum GameOutputs { gameoutput_start, gameoutputloop, gameoutputhold };

int gameOutput(int state) {
  unsigned char displayed_string;
  switch (state) {
    case gameoutput_start: if (start == 0) {state = gameoutput_start;} else {state = gameoutputloop;} break;
    case gameoutputloop:  state = gameoutputloop; break;
    case gameoutputhold:  if (reset == 0) {state = gameoutputhold;} else {state = gameoutput_start; game_over = 0;} break;
    default:  state = gameoutput_start; break;
  }

  switch (state) {
    case gameoutput_start:
      top_row[0] = 0;
      bottom_row[0] = 0;
      for (int i = 0; i < 17; i++) {
        top_row[i] = 0;
        bottom_row[i] = 0;
      }
      break;
    case gameoutputloop:
      //Update position of obstacles
      for (int i = 0; i < 16; ++i) {
        if (top_row[i+1] == 1) {
          top_row[i+1] = 0;
          if (i < 15) {
            LCD_Cursor(i+1+1);
            LCD_WriteData(" ");
          }
          top_row[i] = 1;
          LCD_Cursor(i+1);
          LCD_WriteData('#');
        }
      }

      for (int i = 0; i < 16; ++i) {
        if (bottom_row[i+1] == 1){
          bottom_row[i+1] = 0;
          if (i < 15) {
            LCD_Cursor(i+1+16+1);
            LCD_WriteData(" ");
          }
          bottom_row[i] = 1;
          LCD_Cursor(i+16+1);
          LCD_WriteData('#');
        }
      }
      //Check for contact
      if (top_row[0] == 1 && runnerPos == 0) {
        state = gameoutputhold;
        max_time = timer;
        LCD_ClearScreen();
        LCD_DisplayString(2, "Game Over Press Reset to start");
        //LCD_DisplayString(17, "Time:");
        //LCD_DisplayString(22, max_time);
        game_over = 1;
      }
      else if (bottom_row[0] == 1 && runnerPos == 1) {
        state = gameoutputhold;
        max_time = timer;
        LCD_ClearScreen();
        LCD_DisplayString(2, "Game Over Press Reset to start");
        //LCD_DisplayString(17, "Time:");
        //LCD_DisplayString(22, max_time);
        game_over = 1;
      } else {
        state = gameoutputloop;
      }
      //Remove values at position 0 of each row
      top_row[0] = 0;
      LCD_Cursor(1);
      LCD_WriteData(" ");
      bottom_row[0] = 0;
      LCD_Cursor(17);
      LCD_WriteData(" ");
      //Read opponent Joystick input
      if (Joystick2_position == J2_LEFT) {
        bottom_row[16] = 1;
        //LCD_Cursor(32);
        //LCD_WriteData('#');
      }
      if (Joystick2_position == J2_RIGHT) {
        top_row[16] = 1;
        //LCD_Cursor(16);
        //LCD_WriteData('#');
      }
      break;
    case gameoutputhold: break;
  }
  return state;
}
//------------------------------Task 5------------------------------------------
enum Timer_states {timer_start, timer_loop};

int Timer(int state) {
  switch (state) {
    case timer_start: if (start == 0) {state = timer_start;} else {state = timer_loop;} break;
    case timer_loop:  if (reset == 0) {state = timer_loop;} else {state = timer_start;} break;
      default:  state = output_loop; break;
  }

  switch (state) {
    case timer_start: timer = 0; break;
    case timer_loop:
        timer = timer + 1;
      break;
  }
  return state;
}
//-------------------------------Task 6-----------------------------------------
enum Menu_states {menu_read, menu_display, menu_loop, menu_hold};

int menu(int state) {
  unsigned char high_score_string = 0;
  switch (state) {
    case menu_read: state = menu_display; break;
    case menu_display: state = menu_loop; break;
    case menu_hold: if (reset == 0) {state = menu_hold;} else {state = menu_read;} break;
    case menu_loop: if (start == 0) {state = menu_loop;} else { state = menu_hold; LCD_ClearScreen(); } break;
    default: state = menu_read; break;
  }

  switch (state) {
    case menu_read:
      high_score = eeprom_read_byte(0x0);
      if (high_score > 0x39 || high_score < 0x30) {
        high_score = 0x31;
      }
      break;
    case menu_display:
      LCD_ClearScreen();
      LCD_DisplayString(2, "Welcome        Highest Level:");
      LCD_Cursor(32);
      LCD_WriteData(high_score);
      break;
    case menu_loop:

      break;
    case menu_hold:
      break;
  }
  return state;
}
//------------------------------Task 7------------------------------------------
enum Store_High_Score_states {hs_start, hs_check, hs_hold};

int High_score_check(int state) {
  switch (state) {
    case hs_start:  if (game_over == 0) {state = hs_start;} else {state = hs_check;} break;
    case hs_check:  state = hs_hold; break;
    case hs_hold: if (reset == 0) {state = hs_hold;} else {state = hs_start;} break;
    default: state = hs_start; break;
  }

  switch (state) {
    case hs_start: break;
    case hs_hold: break;
    case hs_check:
      if (max_time > (high_score - 0x30)*10) {
        if (max_time < 10) {
          eeprom_write_byte(0x0, 0x31);
        } else if (max_time < 20) {
          eeprom_write_byte(0x0, 0x32);
        } else if (max_time < 30) {
          eeprom_write_byte(0x0, 0x33);
        } else if (max_time < 40) {
          eeprom_write_byte(0x0, 0x34);
        } else if (max_time < 50) {
          eeprom_write_byte(0x0, 0x35);
        } else if (max_time < 60) {
          eeprom_write_byte(0x0, 0x36);
        } else if (max_time < 70) {
          eeprom_write_byte(0x0, 0x37);
        } else if (max_time < 80) {
          eeprom_write_byte(0x0, 0x38);
        } else if (max_time < 90) {
          eeprom_write_byte(0x0, 0x39);
        } else {
          eeprom_write_byte(0x0, 0x24);
        }
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
  DDRC = 0xFC; PORTC = 0x03; //Output
  DDRD = 0xFF; PORTD = 0x00; //Output

  static task task1, task2, task3, task4, task5, task6, task7;
  task *tasks[] = { &task1, &task2, &task3, &task4, &task5, &task6, &task7 };
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
  //Task 3
  task3.state = start;
  task3.period = 10;
  task3.elapsedTime = task3.period;
  task3.TickFct = &playerInput;
  //Task 4
  task4.state = start;
  task4.period = 150;
  task4.elapsedTime = task4.period;
  task4.TickFct = &gameOutput;
  //Task 5
  task5.state = start;
  task5.period = 1000;
  task5.elapsedTime = task5.period;
  task5.TickFct = &Timer;
  //Task 6
  task6.state = start;
  task6.period = 1000;
  task6.elapsedTime = task6.period;
  task6.TickFct = &menu;
  //Task 7
  task7.state = start;
  task7.period = 200;
  task7.elapsedTime = task7.period;
  task7.TickFct = &High_score_check;

  unsigned long GCD = tasks[0]->period;
  for (int i = 1; i < numTasks; i++) {
    GCD = findGCD(GCD, tasks[i]->period);
  }
  TimerSet(GCD);
  TimerOn();
  //Extra Setup
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
