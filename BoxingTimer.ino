
#include <LiquidCrystal.h>
#include "ScrollableMenu.h"
#include "notes.h"

#define uchar unsigned char

/********* Main Program Defines **********/
/*****************************************/
#define TIMER_MODE_COUNTDOWN 0
#define TIMER_MODE_DISABLED 1

#define REGIME_MENU     0
#define REGIME_WORKOUT  1

#define MENUS_COUNT 6
#define WARNING_SIGNAL_LIMIT_SECONDS 10

#define MUSIC_WARN   1
#define MUSIC_SWITCH 2
#define MUSIC_END    3
/*****************************************/

/********** LCD Keypad Defines ***********/
/*****************************************/
#define KEYPAD_NONE -1
#define KEYPAD_RIGHT 0
#define KEYPAD_UP 1
#define KEYPAD_DOWN 2
#define KEYPAD_LEFT 3
#define KEYPAD_SELECT 4
/*****************************************/

/************** Classes ******************/
/*****************************************/

class LCDKeypad: public LiquidCrystal {
  public:
    LCDKeypad();
    int button();
};

/*****************************************/

class MenuItem {
  public:
    char *name;
    uchar countOfIntrvals;
    uchar currentInterval;
    uchar *intervals;
    uchar cycle;  //if == 0 - do not loop (end at intervals)
  
    MenuItem(char *n, uchar cnt, uchar c, uchar *intvrls);
    void incCurrInterval();
    uchar getCurrIntervalTime();
};

/*****************************************/

uchar boxingIntervals[] =         {60, 240};
uchar workout3Intervals[] =       {60, 60, 60, 30, 60, 15};
uchar workout4Intervals[] =       {60, 60, 60, 45, 60, 30, 60, 15};
uchar fourPlus2Intervals[] =      {12, 240, 60, 240, 60, 240, 60, 240, 60, 60, 60, 45, 60, 30, 60, 15};                    //4 rounds + 2 workouts
uchar fourPlus2Plus1Intervals[] = {12, 240, 60, 240, 60, 240, 60, 240, 60, 60, 60, 45, 60, 30, 60, 15, 60, 240};           //4 rounds + 2 workouts + 1 round
uchar fourPlus2Plus2Intervals[] = {12, 240, 60, 240, 60, 240, 60, 240, 60, 60, 60, 45, 60, 30, 60, 15, 60, 240, 60, 240};  //4 rounds + 2 workouts + 2 rounds

MenuItem menu[MENUS_COUNT] = {
  MenuItem("Boxing", 2, 1, boxingIntervals),
  MenuItem("Workout3", 6, 0, workout3Intervals),
  MenuItem("Workout4", 8, 0, workout4Intervals),
  MenuItem("4 + 2", 16, 0, fourPlus2Intervals),
  MenuItem("4 + 2 + 1", 18, 0, fourPlus2Plus1Intervals),
  MenuItem("4 + 2 + 2", 20, 0, fourPlus2Plus2Intervals)
};
char currentWorkout = 0;

char timerMode = TIMER_MODE_DISABLED;
char regime = REGIME_MENU;
unsigned int seconds = 0;

LCDKeypad lcd;
ScrollableMenu scrollableMenu(2, MENUS_COUNT, drawMenu);

/************ Class' Methods *************/
/*****************************************/

LCDKeypad::LCDKeypad() : LiquidCrystal(8, 9, 4, 5, 6, 7){}

int LCDKeypad::button(){
  static int NUM_KEYS=5;
  static int adc_key_val[5] ={30, 150, 360, 535, 760};
  int k, input;
  input=analogRead(0);
  for (k = 0; k < NUM_KEYS; k++){
    if (input < adc_key_val[k]){
      return k;
    }
  }
  if (k >= NUM_KEYS){
    k = -1;     // No valid key pressed
  }
  return k;
}

/*****************************************/

MenuItem::MenuItem(char *n, uchar cnt, uchar c, uchar *intvrls) {
  name = n;
  countOfIntrvals = cnt;
  intervals = intvrls;
  cycle = c;
  currentInterval = 0;
}

void MenuItem::incCurrInterval() {
  ++currentInterval;
  if(currentInterval >= countOfIntrvals){
    currentInterval = 0;
  }
}

uchar MenuItem::getCurrIntervalTime() {
  return intervals[currentInterval];
}

/*****************************************/

/************ Main Routines **************/
/*****************************************/

void drawMenu(int menuItem, int row, char isActive){
  clearRow(row); 
  lcd.setCursor(0, row);
  if(isActive) {
    lcd.print(">");
  } else {
    lcd.print(" ");
  }
  lcd.print(menu[menuItem].name);
}

void showTime(){
  clearScreen();
  unsigned int localSec = seconds;
  lcd.setCursor(0, 0);
  lcd.print(menu[currentWorkout].name);
  lcd.setCursor(0, 1);
  lcd.print((int)(localSec / 60));
  lcd.print(":");
  if(localSec % 60 < 10) {
    lcd.print("0");
  }
  lcd.print(localSec % 60);
}

void clearScreen() {
  clearRow(0);
  clearRow(1);
}

void clearRow(int row) {
  lcd.setCursor(0, row);
  lcd.print("                ");
}

void sound(int type){
  switch(type) {
    case MUSIC_WARN:
      tone(1, NOTE_B4, 1000);
      break;
    case MUSIC_SWITCH:
      tone(1, NOTE_CS6, 3000);
      break;
    case MUSIC_END:
      tone(1, NOTE_GS3, 5000);
      break;
    
  }
}

void initTimer1(int seconds){
    // initialize timer1 
    noInterrupts();           // disable all interrupts
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;

    // Set timerVal to the correct value for our interrupt interval
    //16MHz / prescaller / wanted_frequency
    int timerVal = (F_CPU / 1024) * seconds;
    if(timerVal > 65535) {  //if more than maximum, than set to maximum
        timerVal = 65535;
    }
    
    OCR1A = timerVal;                       //compare register
    TCCR1B |= (1 << WGM12);                 // CTC moge
    TCCR1B |= (1 << CS12) | (1 << CS10);    // 65535 prescaler 
    TIMSK1 |= (1 << OCIE1A);                // enable timer compare interrupt
    interrupts();                           // enable all interrupts
}

ISR(TIMER1_COMPA_vect) {        // interrupt service routine 
    if(timerMode != TIMER_MODE_DISABLED){
        //countdown seconds
        if(seconds > 0){
          seconds--;
        } else {
          //disableTimer1Interrupts();
          int currInterval = menu[currentWorkout].currentInterval;
          int soundToPlay = MUSIC_SWITCH;
          if(!menu[currentWorkout].cycle && menu[currentWorkout].countOfIntrvals == currInterval - 1) {
            soundToPlay = MUSIC_END;
            timerMode = TIMER_MODE_DISABLED;
          } else {
            menu[currentWorkout].incCurrInterval();
            seconds = menu[currentWorkout].getCurrIntervalTime();
          }
          sound(soundToPlay);
          //enableTimer1Interrupts();
        }
        
        if(seconds == WARNING_SIGNAL_LIMIT_SECONDS){
          //sound warning
          //disableTimer1Interrupts();
          sound(MUSIC_WARN);
          //enableTimer1Interrupts();
        }
    }
    if(regime != REGIME_MENU) {
      showTime();
    }
}

/*void enableTimer1Interrupts() {
  noInterrupts();
  TIMSK1 |= (1 << OCIE1A);
  interrupts();
}

void disableTimer1Interrupts() {
  noInterrupts();
  TIMSK1 = 0;
  interrupts();
}*/

void setup() {
    lcd.begin(16, 2);
    scrollableMenu.draw();
    initTimer1(1);
}

void loop() {
   int key = KEYPAD_NONE, prevKey = KEYPAD_NONE;
   while(1) {
     key = lcd.button();
     if(key != KEYPAD_NONE && key != prevKey){
       if(regime == REGIME_WORKOUT) {
         regime = REGIME_MENU;
         scrollableMenu.draw();
       } else {
         switch(key) {
           case KEYPAD_UP:
              scrollableMenu.keyUp();
              scrollableMenu.draw();
              break;
          case KEYPAD_DOWN:
              scrollableMenu.keyDown();
              scrollableMenu.draw();
              break;
          case KEYPAD_LEFT:  //quit menu
              regime = REGIME_WORKOUT;
              showTime();
              break;
          case KEYPAD_RIGHT:
          case KEYPAD_SELECT:
              timerMode = TIMER_MODE_DISABLED;
              currentWorkout = scrollableMenu.getCurrent();
              menu[currentWorkout].currentInterval = 0;
              seconds = menu[currentWorkout].intervals[0];
              regime = REGIME_WORKOUT;
              timerMode = TIMER_MODE_COUNTDOWN;
              showTime();
              break;
         }
       }
     }
     prevKey = key;
     delay(50);
   }
}
