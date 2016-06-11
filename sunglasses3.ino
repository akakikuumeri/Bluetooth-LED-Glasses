#include <digitalWriteFast.h>

#define DELAY 200
#define INTERRUPT 15 //millisecondsish
#define MIDDLEGAP 4 //how many invisible pixels in the middle of the glasses
#include "font.h"
//#include <TimerOne.h>

//Pin connected to ST_CP of 74HC595
int latchPin = 12;
//Pin connected to SH_CP of 74HC595
int clockPin = 11;
////Pin connected to DS of 74HC595
int dataPin = 13;

int row = 0;
int rowpin[9] = {2,3,4,5,6,7,8,9,10};

String text = "EEIC Bikkuri Dokkiri Mecha!";
int textl = 27;
boolean waittoscroll = false;

unsigned long currenttime = 0; //incremented every interrupt. millis() doesn't work well with interrupts

char buffer0[3*9] = {
  B11111111,B11111111,B11111111,
  B11100000,B11110000,B01111111,
  B10010100,B01100000,B00011111,
  B10011100,B01100000,B00011111,
  B10111110,B11110000,B00011111,
  B10111110,B11110000,B00011111,
  B10011101,B11111000,B00011111,
  B11001011,B11111100,B00111111,
  B11111111,B11111111,B11111111
};
char buffer1[3*9] = {
  B11111111,B11111111,B11111111,
  B11100000,B11110000,B01111111,
  B10010100,B01100010,B10011111,
  B10011100,B01100111,B11011111,
  B10111110,B11110111,B11011111,
  B10111110,B11110111,B11011111,
  B10011101,B11111011,B10011111,
  B11001011,B11111101,B00111111,
  B11111111,B11111111,B11111111
};
char *image = buffer0;/*[9*20]={
B11111111,B1111111,B11111111,
B11111111,B1111111,B11111111,
B11111111,B1111111,B11111111,
B11111111,B1111111,B11111111,
B11111111,B1111111,B11111111,
B11111111,B1111111,B11111111,
B11111111,B1111111,B11111111,
B11111111,B1111111,B11111111,
B11111111,B1111111,B11111111
};*/
int currentbuffer = 0;

long timer =0;
int timerinterval = 20;
boolean timerflag = true;
int scrollx = 0;
#define TEXT 1
#define EYES 0
#define LINES 2
#define DIAGONAL 3
#define SINE 4
#define RANDOM 5
#define SHINE 6
#define DEBUG 7
#define SMALL 8
#define MAXPROGRAM 5
#define MULTIPLEXDEMOSPEED 40
#define LETTERWIDTH 7
int DEBOUNCETIME = 300;
unsigned long buttondebounce = 0;
int program = EYES;
int multiplexdemocounter = 0;

String voltage = "88";

void setup() {
  //set pins to output so you can control the shift register
  //text.reserve(200);
  
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  for (int row = 0; row <9; row++) {
    pinModeFast(rowpin[row],OUTPUT);
    digitalWriteFast(rowpin[row],LOW);
  }
  
    // initialize Timer1
    cli();          // disable global interrupts
    TCCR1A = 0;     // set entire TCCR1A register to 0
    TCCR1B = 0;     // same for TCCR1B
 
    // set compare match register to desired timer count:
    OCR1A = INTERRUPT;
    // turn on CTC mode:
    TCCR1B |= (1 << WGM12);
    // Set CS10 and CS12 bits for 64 prescaler:
    TCCR1B |= (1 << CS10);
    TCCR1B |= (1 << CS12);
    // enable timer compare interrupt:
    TIMSK1 |= (1 << OCIE1A);
    sei();          // enable global interrupts
    
//  Timer1.initialize(INTERRUPT);
//  Timer1.attachInterrupt(update_image);
//  Timer1.start();
  Serial.begin(19200);//for some reason it seems to half this
}

ISR(TIMER1_COMPA_vect)
{//timer one
    update_image();
}

void loop() {
  //ready to take in new text
  while (waittoscroll == false &&Serial.available()){
    char inChar = (char)Serial.read(); 
    if (inChar == '@') {
      waittoscroll = true;
      return;
    }
    // add it to the inputString:
    text += inChar;
    textl++;
    
  }
  
  // count from 0 to 255 and display the number 
  // on the LEDs
  if (currenttime - buttondebounce > DEBOUNCETIME) { if (analogRead(A7) > 128) {
     buttondebounce = currenttime;
      program = (program+1)%(MAXPROGRAM+1);
      timer = currenttime;
      timerflag = true;//to force immediate update
  }}
  
  //check time:
  if (currenttime - timer > timerinterval) {
        timerflag = true;
        timer = currenttime;
      }
  
  
  switch (program) {
    case TEXT:
    //OCR1A = INTERRUPT;
    DEBOUNCETIME = 300;//return to normal
    timerinterval = 25;
      if (timerflag) {
        timerflag = false;
        scrollx--;
        if (scrollx >20) scrollx = 20;
        if (scrollx < -LETTERWIDTH * textl){
           scrollx = 20;
           if (Serial.available()) {
             waittoscroll = false;
             text = "";
             textl=0;
           }
        }//when out of screen to the left, move to right
        clear_image();
        write_string(text,scrollx);
        //Serial.println(text);
      } break;

    case SINE:
    timerinterval = 5;
    if (timerflag) {
      timerflag = false;
      scrollx+=1;
      if (scrollx > 100) scrollx = 0;
      clear_image();
      draw_sin(scrollx);
      
    }break;
    
    case EYES:
    timerinterval = (scrollx == 0 || scrollx == 4) ? 150 : random(400,800);//make scrollx (blinking) short, otherwise random
    if (timerflag) {
      timerflag = false;
      if (scrollx == 1 || scrollx == 3 || scrollx == 4) {//if looking sideways or blinking
        scrollx = 2;//look foward
      } else if (scrollx == 2 && random(8)==0) {//if looking foward and small chance
        scrollx = 4;//winking
      } else {
        scrollx=random(4);//use scrollx as frame counter
        if (scrollx == 0) timerinterval = 150;
      }
      clear_image();
      draw_eyes(scrollx);
      
    }break;
    
    case LINES:
    timerinterval = 33;
    if (timerflag) {
      timerflag = false;
      scrollx++;//use scrollx as frame counter
      if (scrollx >=12) scrollx = 0;
      if (scrollx <0) scrollx = 0;
      clear_image();
      draw_lines(scrollx);
      
    }break;
    
    case DIAGONAL:
    timerinterval = 33;
    if (timerflag) {
      timerflag = false;
      scrollx+=1;
      if (scrollx >=12) scrollx = 0;
      if (scrollx <0) scrollx = 0;
      clear_image();
      draw_diagonal(scrollx);
      
    }break;
    
    case RANDOM:
    timerinterval = 100;
    if (timerflag) {
      timerflag = false;
      clear_image();
      draw_noise();
      
    }break;
    
    case SHINE:
    timerinterval = 5;
    if (timerflag) {
      timerflag = false;
      scrollx+=1;
      if (scrollx >=3000) scrollx = 0;
      if (scrollx <0) scrollx = 0;
      clear_image();
      draw_shine(scrollx); 
    }break;
    
    case DEBUG:
    timerinterval = 1;
    //OCR1A = 300;//slow mo to demo multiplexing
      if (timerflag) {
        timerflag = false; 
        /*clear_image();
        voltage = String(readVcc());*/
        /*write_letter('E',1);
        write_letter('E',5);
        write_letter('I',9);
        write_letter('C',13);//voltage[1],2);*/
        clear_image();
        draw_eyes(2);//normal eyes
        //write_letter(//voltage[2],12);
        //Serial.println(text);
      } break;
    
    /*case SMALL:
    timerinterval = 30;
      if (timerflag) {
        timerflag = false;
        scrollx--;
        if (scrollx < -5 * textl){
           scrollx = 20;
           if (Serial.available()) {
             waittoscroll = false;
             text = "";
             textl=0;
           }
        }//when out of screen to the left, move to right
        clear_image();
        write_string_small(text,scrollx);
        //Serial.println(text);
      } break;*/
  }
      
      
  //update_image();
}


void update_image() {
  currenttime++;
  if (program == DEBUG && multiplexdemocounter > 0) {
    //dont go yet, slow down to show multiplexing demo
    multiplexdemocounter--;
  } else {
    if (program == DEBUG) { multiplexdemocounter = MULTIPLEXDEMOSPEED;}
  row = (row+1)%9;
    // take the latchPin low so 
    // the LEDs don't change while you're sending in bits:
    digitalWriteFast(latchPin, LOW);
    // shift out the bits:
    char *bufptr = currentbuffer ? buffer0 : buffer1;
    shiftOut(dataPin, clockPin, LSBFIRST, bufptr[row*3+2]);  
    shiftOut(dataPin, clockPin, LSBFIRST, bufptr[row*3+1]);  
    shiftOut(dataPin, clockPin, LSBFIRST, bufptr[row*3]);   
    
    //turn off the previous row
    
      digitalWriteFast(rowpin[(row == 0? 8 : row-1)], LOW);
  
    //take the latch pin high so the LEDs will light up:
    digitalWriteFast(latchPin, HIGH);
    
    //turn this row on:     
      digitalWriteFast(rowpin[row], HIGH);
    //}
    // pause before next value:
    //delayMicroseconds(DELAY);
  }
}
void write_string(String string, int x) {
  int i = 0;
  while (text[i] != '\0') {
    write_letter(string[i], x + i*LETTERWIDTH);
    i++;
  }
  swap_buffers();
}

void write_string_small(String string, int x) {
  int i = 0;
  while (text[i] != '\0') {
    write_letter_small(string[i], x + i*5);
    i++;
  }
  swap_buffers();
}
  
void write_letter(char letter, int x) {
   int whichbyte;
   int offset = 0; //offsrt for the gap between eyes
   if (x >= 19 || x <= -7) return; //if out of bounds
   for (int row = 0; row < 8; row ++) {
     offset = 0;
     for (int pixel = 0; pixel < 7; pixel++) {//go through each pixel
       //if ((7-pixel) + x > 9 + MIDDLEGAP) offset = 0;//-MIDDLEGAP;
       if (!(x > 15 && pixel == 6)) {//this case will overflow to the next row, showing annoying extra pixel at left edge
         whichbyte = (row + 1) * 3 //start writing from second row from top.
               + (x + pixel) / 8; //choose the correct byte from the 3
         bitWrite(image[whichbyte], 7 - (pixel + x)%8, (font[letter][row] >> pixel) & B00000001 == B00000001);
       }
     }
      //only write in first byte for test for now
      //image[row*3+3] = font[letter][row] >> x;
   }
   
}

void write_letter_small(char letter, int x) {
  //noInterrupts();
  int whichbyte;
  int offset = 0;
  if (x >= 20/*+MIDDLEGAP*/ || x <= -5) return; //if out of bounds
   for (int row = 0; row < 7; row ++) {
     offset = 0;
     for (int pixel = 0; pixel < 5; pixel++) {//go through each pixel
       //if (pixel + x > 9 + MIDDLEGAP) offset = -MIDDLEGAP;
       whichbyte = (row + 1) * 3 //start writing from second row from top.
             + (x + pixel/* + offset*/) / 8; //choose the correct byte from the 3
       bitWrite(image[whichbyte], 7 - (x + pixel/* + offset*/)%8, (smallfont[(letter-32)*5 + pixel] >> row) & B00000001 == B00000001);
     }
      //only write in first byte for test for now
      //image[row*3+3] = font[letter][row] >> x;
   }
   //interrupts();
}

void clear_image() {
  for (int i; i < 3*9; i++) {
    image[i] = 0;
  }
}

void swap_buffers() {
  if (currentbuffer == 0) {
      currentbuffer = 1;
      image = buffer1;
    } else {
      currentbuffer = 0;
      image = buffer0;
    }
}

void draw_noise() {
  for (int b = 0; b < 27; b++) {
        image[b] = (char)random(256);
      }
   swap_buffers();
}

void draw_sin(int offset) {
  for (int b = 0; b < 27; b++) {
        for (int j = 0; j < 8; j++) {//for each bit
          int x = (b%3)*8 + j;
          int y = b / 3;
          if (sin((x+offset)*0.5) > y/3.5-1.25) {
            bitSet(image[b],7-j);
          }
        }
      }
   swap_buffers();
}

void draw_eyes(int frame) {
  char framesonly[3*9] = {//frames
  B11111111,B11111111,B11111111,
  B11100000,B11110000,B01111111,
  B10000000,B01100000,B00011111,
  B10000000,B01100000,B00011111,
  B10000000,B11110000,B00011111,
  B10000000,B11110000,B00011111,
  B10000001,B11111000,B00011111,
  B11000011,B11111100,B00111111,
  B11111111,B11111111,B11111111
  };
  
  for (int i = 0; i < 3*9; i++) {
    image[i] = framesonly[i];//copy array
  }
  int whichbyte;
  //next, the eyes themselves:
  if (frame == 0) { //eyes closed
    image[3*5] = B11111111;
    image[3*5+1] = B11111111;
    image[3*5+2] = B11111111;
  } else if (frame == 4) {//blinking
  //left eye, normal
     for (int row = 1; row < 7; row ++) {
       for (int pixel = 0; pixel < 3; pixel++) {//go through each pixel
         whichbyte = row * 3;
         if (!((row == 6 || row == 1) && pixel != 1)) {//on the 1st and 6th row, only draw middle pixel to give tapered edge
           bitSet(image[whichbyte], pixel+1);
         }
       }
     }
     //right eye, closed
     image[3*5+1] = B11111111;
    image[3*5+2] = B11111111;
  } else {//normal
    //left eye
     for (int row = 1; row < 7; row ++) {
       for (int pixel = 0; pixel < 3; pixel++) {//go through each pixel
         whichbyte = row * 3;
         if (!((row == 6 || row == 1) && pixel != 1)) {//on the 1st and 6th row, only draw middle pixel to give tapered edge
           bitSet(image[whichbyte], frame+pixel-1);
         }
       }
     }
     //right eye
     for (int row = 1; row < 7; row ++) {
       for (int pixel = 0; pixel < 3; pixel++) {//go through each pixel
         whichbyte = row * 3 +1 + (pixel + frame == 1);//if looking right, one line flows over to next byte
         if (!((row == 6 || row == 1) && pixel != 1)) {//on the 1st and 6th row, only draw middle pixel to give tapered edge
           bitSet(image[whichbyte], (frame+pixel-2)>=0 ? frame+pixel-2: 7);
         }
       }
     }
  }
  
  //image = framesonly;
  swap_buffers();
}

void draw_lines(int offset) {
  for (int i=0; i<9; i++) {
    int row = (i+offset)%12;//loop around
    if ((row/3)%2 == 1) {
      image[i*3] = B11111111;
      image[i*3+1] = B11111111;
      image[i*3+2] = B11111111;
    }
  }
  swap_buffers();
}

void draw_diagonal(int offset) {
  for (int b = 0; b < 27; b++) {
        for (int j = 0; j < 8; j++) {//for each bit
          int x = (b%3)*8 + j;
          int y = b / 3;
          if (x > 9) x+= 2; //because of the gap between eyes
          if (((x+offset+y)/3)%2 == 0 ) {
            bitSet(image[b],7-j);
          }
        }
      }
   swap_buffers();
}

void draw_shine(int offset) {//shows totally white glasses, but occasionally a shining stripe slashes across
  for (int b = 0; b < 27; b++) {
        for (int j = 0; j < 8; j++) {//for each bit
          int x = (b%3)*8 + j;
          int y = b / 3;
          if (x > 9) x+= 2; //because of the gap between eyes
          if (((20-x/2+offset+y)/4)%50 != 0 ) {
            bitSet(image[b],7-j);
          }
        }
      }
   swap_buffers();
}

long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  
 
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring
 
  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both
 
  long result = (high<<8) | low;
 
  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}
