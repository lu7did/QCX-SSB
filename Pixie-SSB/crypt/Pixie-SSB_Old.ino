//*--------------------------------------------------------------------------------------------------
//* PixieSSB Firmware Version 1.0
//*--------------------------------------------------------------------------------------------------
//* This is an Arduino controlled, Pixie based, HF 7 MHz transceiver firmware
//* Features:
//*    Modes: CW-SSB (USB/LSB), perhaps AM & FM too in the future
//*    Direct conversion receiver
//*    Direct PLL synthesis SSB generation (inspired by Guido PE1NNZ work)
//*    LCD 16x2 & Rotary encoder support
//* Only for radioamateur uses, commercial usage prohibited without license
//* Copyright 2020 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
//* Plataforma: Arduino NANO/UNO/Mega
//* LCD: 16x2 HD44780 or equivalent
//*   
//*   D0 - RxD  (reserved)
//*   D1 - TxD  (reserved)
//*
//*   Rotary encoder handler
//*   D2 - Encoder (A)
//*   D3 - Encoder (B)
//*
//*   LCD Handling
//*   D4 - DB4
//*   D5 - DB5
//*   D6 - DB6
//*   D7 - DB7
//*   D8 - E
//*   D9 - RW
//*   D10- BackLigth
//*
//*   AD9850
//*   D11 - ADS9850 (W_CLK)
//*   D12 - ADS9850 (FQ_UD)
//*   D13 - ADS9850 (in)
//*   A1  - AD9850  (RESET)
//*
//*   Transceiver control
//*   A0  - KEY_OUT (out)
//*   **  - KEY_OUT (out)
//*   A2  - Microphone/DIT (in)
//*   A3  - BUTTONS
//*   A4  - PTT
//*   A5  - Not Used
//*-------------------------------------------------------------------------------------------------------
//*----- Program Reference data
//*-------------------------------------------------------------------------------------------------------
//*-------- Copyright and Program Build information
#define PROGRAMID "Pixie"
#define COPYRIGHT "(c)LU7DID 2021"
#define PROG_VERSION "1.0"
#define PROG_BUILD "000"
//*----------------------------------------------------------------------------------
//*  System Status Word
//*----------------------------------------------------------------------------------
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define ln(x) (log(x)/log(2.718281828459045235f))

//#define KEY
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//*                     PROGRAM AND STATUS MANAGEMENT
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//*--- Master System Word (MSW)

#define CMD       B00000001
#define GUI       B00000010
#define PTT       B00000100
#define MENU      B00001000
#define DIR       B00010000
#define LCLK      B00100000
//#define ADC       B01000000
#define BCK       B10000000

//*----- Master Timer and Event flagging (TSW)

#define FT1       B00000001
#define FT2       B00000010
#define FT3       B00000100
#define FT4       B00001000
#define FT5       B00010000
#define FCLOCK    B00010000
#define FTS       B00100000
#define FDOG      B01000000
#define FBCK      B10000000

//*----- UI Control Word (USW)

#define BUSY      B00000001
#define BMULTI    B00000010
#define BCW       B00000100
#define BCCW      B00001000
#define RXTX      B00010000
//#define ADC       B00100000
#define READY     B00100000
#define KDOWN     B01000000
#define FFT       B10000000
 
//#define BUSY      B10000000       //* Used for Squelch Open in picoFM and for connected to DDS on sinpleA
//#define CONX      B10000000

#define ZERO             0
#define VOLUME           5

//*----- Joystick Control Word (JSW)

#define JLEFT     B00000001
#define JRIGHT    B00000010
#define JUP       B00000100
#define JDOWN     B00001000
#define JCLICK    B00010000

typedef struct {
  byte ones;
  byte tens;
  byte hundreds;
  byte thousands;
  byte tenthousands;
  byte hundredthousands;
  byte millions;
} FSTR;


//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//*              DELAY DEFINITIONS (SOME NOT USED)
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
#define DELAY_DISPLAY 4000     //delay to show the initial display in mSecs
#define DELAY_SAVE    1000     //delay to consider the frequency needs to be saved in mSecs
#define LCD_DELAY     1000
#define CMD_DELAY      100
#define PTY_DELAY      200
#define DIM_DELAY    30000
#define DOG_DELAY    60000
#define BLINK_DELAY   1000
#define SAVE_DELAY    2000
#define EEPROM_COOKIE  0x1f
#define SERIAL_MAX      16
#define MAXQUEUEADC  16
#define FORCEFREQ     0
#define LCD_ON        1
#define LCD_OFF       0
#define QUEUEMAX     16        // Queue of incoming characters thru serial port

//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//*                     TRANSCEIVER DEFINITIONS
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
#define VFOA    0
#define VFOB    1
#define VFO_STEP_1Hz            1
#define VFO_STEP_10Hz          10
#define VFO_STEP_100Hz        100
#define VFO_STEP_1KHz        1000
#define VFO_STEP_5KHz        5000
#define VFO_STEP_10KHz      10000
#define VFO_STEP_100KHz    100000
#define VFO_STEP_1MHz     1000000
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//*                       TRANSCEIVER CONTROL
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
#define VFOMAX                  2    //* Number of VFO
#define VFO_DEFAULT       7030000    //* Default starting frequency
#define VFO_HIGHER        7300000    //* Default upper band limit
#define VFO_LOWER         7000000    //* Default lower band limit
#define VFO_SHIFT             600    //* Default CW shift (tone)
#define VFO_RIT               100    //* Default RIT +/- increment

#define DEBUG                   1
//#define CAT                     1

//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//*                       LIBRARY AND INCLUDES
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
#include <LiquidCrystal.h>
#include <stdio.h>
#include <EEPROM.h>
//#include <si5351.h>
#include <Wire.h>
#include <SPI.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//*              ENCODER AND JOYSTICK DEFINITION
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
#define Encoder_A 3
#define Encoder_B 2
#define LCD_D4    4
#define LCD_D5    5
#define LCD_D6    6
#define LCD_D7    7
#define LCD_RS    8
#define LCD_EN    9
#define LCD_BACK  10

#define KEY_OUT  A0
#define DIT      A4
#define BUTTONS  A3
#define MIC      A2

//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//*              MAIN OBJECT DEFINITION
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);  // LCD handling system     

//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//*                       APPLICATION TIMERS
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
uint16_t T1=0;
uint16_t T2=0;
uint16_t T3=0;
uint16_t T4=0;
uint16_t T5=0;
uint16_t TV=0;
uint16_t TS=0;
uint16_t TDOG=0;
uint16_t TBCK=0;
uint16_t TDIM=0;

//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//*                       APPLICATION SYSTEM STATUS WORDS
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
volatile byte MSW = 0;
volatile byte TSW = 0;
volatile byte USW = 0;
volatile byte JSW = 0;

//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//*                       SERIAL PORT MANAGEMENT
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
char hi[40];

#ifdef CAT
byte rxBuffer[16];

#endif
int  n=0;
int memstatus = 1;          // value to notify if memory is current or old. 0=old, 1=current.
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//*                       VFO MANAGEMENT
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
volatile int32_t  freq = 7030000;
volatile uint32_t vfo;
volatile uint32_t vfostep  = VFO_STEP_1KHz;
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//*              FT817 CAT Management
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
#define MLSB   0x00
#define MUSB   0x01
#define MCW    0x02
#define MCWR   0x03
#define MAM    0x04
#define MWFM   0x06
#define MFM    0x08
#define MDIG   0x0A
#define MPKT   0x0C
#define SMETERMAX 14

//*--- Transmission control

#define VOX    0B10000000
#define AGC    0B01000000
#define SHIFT  0B00100000
#define RIT    0B00010000
#define LOCK   0B00001000
#define PTT    0B00000100
#define SPLIT  0B00000010
#define VFO    0B00000001

byte FT817= 0x00;
byte mode=MUSB;

int cnt=0;
unsigned int Encoder_number=0;      
void Encoder_san();

/*
 * ADC & FFT Areas
 */

#define SAMPLES   128
#define QUEUE     140

volatile int adcCount=0;
volatile int fftCount=0;

int          data[SAMPLES+10];
int          adc[QUEUE+10];

unsigned int Pow2[13]={1,2,4,8,16,32,64,128,256,512,1024,2048}; // declaring this as global array will save 1-2 ms of time


//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//*              LOCAL LIBRARIES
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
#include "UtilLib.h"
#include "ADS9850Lib.h"
#include "MemSizeLib.h"
#include "CATLib.h"
#include "LCDLib.h"
#include "Quick_FFT.h"


//*--------------------------------------------------------------------------------------------
//* managePTT
//* check if the PTT is set
//*--------------------------------------------------------------------------------------------
void managePTT() {
  int nowPTT = digitalRead(DIT);
 
  if (nowPTT != (int) getWord(FT817,PTT) && getWord(TSW,FT4)==false) {
     setWord(&FT817,PTT,(boolean)nowPTT);
     setWord(&USW,RXTX,true);
     setWord(&TSW,FT4,true);
     T4=50;
     sprintf(hi,"PTT=%d (%d)\n",nowPTT,getWord(FT817,PTT));
     Serial.println(hi);
  }
}

//*--------------------------------------------------------------------------------------------
//* rxtx
//* switch between transmission and reception (initial needs further development)
//*--------------------------------------------------------------------------------------------
void rxtx() {

  if (getWord(FT817,PTT)==false) {    //TX activated      
     
     SendFrequency(vfo+(mode==MCW ? VFO_SHIFT : 0));     
     digitalWrite(KEY_OUT,1);    
  } else {       
     SendFrequency(vfo);

     digitalWrite(KEY_OUT,0);     
  }
  showPTT();   
  
}


//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//*                     SETUP
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
void setup() {

//*--- Init serial port and establish handshake with DRA018F 

  Serial.begin(9600);

  pinMode(LCD_BACK,OUTPUT);
  pinMode(Encoder_A, INPUT); 
  pinMode(Encoder_B, INPUT); 
  pinMode(KEY_OUT,OUTPUT);

  //*--- Set KEY LOW (Reception)
  digitalWrite(KEY_OUT,0);

  pinMode(BUTTONS, INPUT_PULLUP);  // rotary button
  pinMode(DIT, INPUT_PULLUP);

  
  
  //*--- LCD type definition and start
  
  lcd.begin(LCD_COLS, LCD_ROWS);   // start the LCD library  
  digitalWrite(LCD_BACK,LCD_ON);
  TDIM=DIM_DELAY;  
  lcd.setCursor(0,0);  

  //*--- Create special characters for LCD

  lcd.createChar(0,MARK);
  lcd.createChar(6,TX);


   //*---- Define decoder channels 
    
  digitalWrite(Encoder_A, 1);
  digitalWrite(Encoder_B, 1);

/*
 * INIT QCX-SSB
 */
  digitalWrite(KEY_OUT, LOW);  // for safety: to prevent exploding PA MOSFETs, in case there was something still biasing them.
  
  MCUSR = 0;

  //========================================
  //* Assign interrupts
  //========================================
  
  pciSetup(Encoder_B);
  pciSetup(DIT);
  

  lcd.setCursor(0, 0);        // Place cursor at [0,0]
  lcd.print(String(PROGRAMID));
  lcd.print(F("\x00"));
  lcd.print(" v"+String(PROG_VERSION));

  lcd.setCursor(0, 1);        // Place cursor at [0,1]
  lcd.print(String(COPYRIGHT));
  delay(DELAY_DISPLAY);
  lcd.clear();


/*
 * Manage Timer Interrupts
 * Timer0 (8 bits) is used to create the audio sampling interrupt (4800 times/sec)
 * Timer1 (16 bits) is used to create a 1 mSeg general purpose interrupt service
 */

#define T_1mSec          65473 //Timer pre-scaler for 1 KHz or 1 msec
#define TIMER_SAMPLERATE 51    

  
  
  noInterrupts(); // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;

/*
 * Program Timer1 and configure it to interrupt on overflow
 * (count up and interrupt when reaching 0xffff
 */
    
  TCNT1 = T_1mSec; // preload timer 65536- [(16000000/256)/f(Hz)] = 34286
  TCCR1B |= (1 << CS12); // 256 prescaler
  TIMSK1 |= (1 << TOIE1); // enable timer overflow interrupt

/* 
 *  Program Timer0 and configure it to interrupt whenever the pattern is found
 *  which should happen SAMPLERATE times per second
 */
  
  TCCR0A = 0; // set entire TCCR2A register to 0
  TCCR0B = 0; // same for TCCR2B
  TCNT0  = 0; //initialize counter value to 0
  
  /* --- Alternate method to compute sample rate programatically
  int sample=((16000000) / (samplerate*64))-1;
  if (sample>=256) {
     sample=256;  //**Error condition
  }
  */
 
  OCR0A = TIMER_SAMPLERATE;        // = (16*10^6) / (SAMPLE_RATE*64) - 1 (must be <256)
  
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  
  // Set CS01 and CS00 bits for 64 prescaler (011)
  TCCR0B |= (1 << CS01) | (1 << CS00);   
  
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);
 
  interrupts(); // enable all interrupts

/*
 * End of Timer Interrupt setup
 *  
 */


/*
 * ADC Interrupt configuration
 */

  ADCSRA &= ~(bit (ADPS0) | bit (ADPS1) | bit (ADPS2)); // clear prescaler bits
  
 
//  ADCSRA |= bit (ADPS0);                               //   2  
//  ADCSRA |= bit (ADPS1);                               //   4  
//  ADCSRA |= bit (ADPS0) | bit (ADPS1);                 //   8  
//  ADCSRA |= bit (ADPS2);                               //  16 
//  ADCSRA |= bit (ADPS1) | bit (ADPS2);                 //  64
//  ADCSRA |= bit (ADPS0) | bit (ADPS1) | bit (ADPS2);   // 128

  ADCSRA =  bit (ADEN);   // turn ADC on
  ADCSRA |= bit (ADPS0) | bit (ADPS2);                   //  32 Pre-scaler
  ADMUX =   bit (REFS0) | (MIC & 0x07);  // AVcc    


//*--- PTT Setup
  setWord(&MSW,CMD,false);
  setWord(&FT817,PTT,true);  //PTT on reception 
   
//*--- set the initial default VFO and the initial frequencies

  setWord(&FT817,VFO,VFOA);
  
  vfo=freq;
  initDDS();
  SendFrequency(vfo);

//*------ Show information at LCD

  showPanel();
  showFreq();

//*--- Print Serial Banner (TEST Mode Mostly)

  setWord(&FT817,PTT,true);
  managePTT();
  setWord(&USW,READY,true);
  setWord(&USW,FFT,false);
  
  rxtx();
  

#ifndef CAT

  sprintf(hi,"%s %s Compiled %s %s",PROGRAMID,PROG_VERSION,__TIME__, __DATE__);
  Serial.println(hi);
  sprintf(hi,"(c) %s",COPYRIGHT);
  Serial.print("RAM Free=");
  Serial.println(freeMemory());

#endif 
  
}

//=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
//*                                              PROGRAM FLOW CONTROL
//=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
//*--------------------------------------------------------------------------------------------------
//* Manages the reading of the KeyShield buttons
//*--------------------------------------------------------------------------------------------------
int readKeyShield() {

int adc_key_in = analogRead(0);
      
//* read the value from the sensor  
//* my buttons when read are centered at these valies: 0, 144, 329, 504, 741  
//* we add approx 50 to those values and check to see if we are close  

if (adc_key_in > 1000) return btnNONE; 

//* Follows approximate limit values for all actions

if (adc_key_in < 50)   return btnLEFT;   
if (adc_key_in < 150)  return btnUP;   
if (adc_key_in < 250)  return btnRIGHT;   
if (adc_key_in < 450)  return btnSELECT;   
if (adc_key_in < 700)  return btnDOWN;     
if (adc_key_in < 850)  return btnEncodeOK;

return btnNONE;  // when all others fail, return this... 
}
//*------------------------------------------------------------------------------------------------------------------------------
//*Handle_Commands
//*Main handler for commands from various functions
//*------------------------------------------------------------------------------------------------------------------------------
void Handle_Commands() {

   int keyVal=readKeyShield();

//*--- Manage rotary encoder, main purpose is to change the frequency

   if (getWord(MSW,CMD)==false) {    //VFO Mode

      if (getWord(USW,BCW)==true) {  //Encoder rotation found CW
         setWord(&USW,BCW,false);
         showMark(">");
         updateVFO(vfostep);
         SendFrequency(vfo);
         showFreq();
         T2=1000;
         return;
      }

      if (getWord(USW,BCCW)==true) {  //Encoder rotation found CW
         setWord(&USW,BCCW,false);
         showMark("<");
         updateVFO(-vfostep);
         SendFrequency(vfo);
         showFreq();
         T2=1000;
         return;
      }

      if (getWord(TSW,FT2)==true) {
         setWord(&TSW,FT2,false);
         unshowMark();
         return;
      }

}

}
/*-----------------------------------------------------------------------------------------------------------
 *                                       LOOP
 *-----------------------------------------------------------------------------------------------------------*/
void loop() {

#ifdef CAT
//*---- Process CAT (if enabled
        get();
#endif

//*---- Processs ADC Samples

     //if (n>16384) {Serial.println("ADC Kick\n");n=0;}

     if (getWord(USW,FFT)==true && getWord(USW,BUSY)==false) {
        setWord(&USW,BUSY,true);
        
        //float f=0.0;
        float f=Q_FFT(data,SAMPLES,4800);
        
        fftCount=0;        
        setWord(&USW,FFT,false);
        
        if (f>=0.0) {    
           SendFrequency(vfo+(int)f);
          /* 
           n++;
           if (n>18) {
             sprintf(hi,"FFT f=%d\n",(int)f);
             Serial.println(hi);
             n=0;
           }
          */ 
           
        }   
        
        setWord(&USW,BUSY,false);

     }
 
//*---- Process performance statistics (DEBUG)

     if (getWord(USW,RXTX)==true) {
         setWord(&USW,RXTX,false);
         rxtx();        
     } else {
    
       Handle_Commands();
     }  
}
//**************************************************************************************************************
//*                                           TIMER & INTERRUPT MANAGEMENT
//**************************************************************************************************************
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//* Encoder_san
//* interrupt handler to manage rotary button
//--------------------------------------------------------------------------------------------------------------------------------------------------------
 void Encoder_san()
{ 
  
     if(digitalRead(Encoder_A) && !digitalRead(Encoder_B))
    {
      Encoder_number++;
      setWord(&USW,BCCW,true);     //bRotaryCW  = true;
    }
    if (digitalRead(Encoder_A) && digitalRead(Encoder_B)) {
       Encoder_number--;
       setWord(&USW,BCW,true);    //bRotaryCCW = true;
    }     
}  
//*--------------------------------------------------------------------------------------------------
//* ADC Interrupt Handler
//*--------------------------------------------------------------------------------------------------
ISR (ADC_vect) {

  
   if (getWord(FT817,PTT)==false && (mode==MLSB || mode==MUSB) && getWord(USW,READY)==true) {
   } else {
     interrupts();
     return; 
   }

  if (adcCount>0 && getWord(USW,FFT)==false && fftCount<SAMPLES) {
     n++;
     for(int i=0;i<adcCount;i++) {
        data[fftCount++]=adc[i];
        if(n>16384){sprintf(hi,"adc[%d]=%d\n",i,adc[i]);Serial.println(hi);n=0;}
     }
     adcCount=0;
     if (fftCount>=SAMPLES) {
        setWord(&USW,FFT,true);
     }
  }


  adc[adcCount++]=(ADCH<<8  | ADCL);
 
  if (adcCount>=QUEUE) {
     adcCount=QUEUE-1;
  }

  interrupts();

  
}  // end of ADC_vect
//*--------------------------------------------------------------------------------------------------
//* Timer Interrupt handler
//*      (TIMER0): This is the interrupt handler for TIMER0 set as samplerate
//*--------------------------------------------------------------------------------------------------
ISR(TIMER0_COMPA_vect){//timer0 interrupt samplerate times per second


  if (getWord(FT817,PTT)==false && (mode==MLSB || mode==MUSB) && getWord(USW,READY)==true) {
     ADCSRA |= bit (ADSC) | bit (ADIE);    
      
  }

  interrupts();

}
//*--------------------------------------------------------------------------------------------------
//* Timer Interrupt handler
//*      (TIMER1): This is the interrupt handler for TIMER1 set as 1 msec or 1 KHz
//*--------------------------------------------------------------------------------------------------
ISR(TIMER1_OVF_vect) // interrupt service routine for TIMER1
{

  TCNT1 = T_1mSec; // preload timer

  
//*--- Serve Timer 1 (T1) 
  if (T1>0) { 
      T1--;
      if (T1==0){
         setWord(&TSW,FT1,true);
      }
  }
  if (T2>0) { 
      T2--;
      if (T2==0){
         setWord(&TSW,FT2,true);
      }
  }

    if (T3>0) { 
      T3--;
      if (T3==0){
         setWord(&TSW,FT3,true);
      }
  }
     if (T4>0) { //This is an inverse timer
      T4--;
      if (T4==0){
         setWord(&TSW,FT4,false);
      }
  }
    if (T5>0) { 
      T5--;
      if (T5==0){
         setWord(&TSW,FT5,true);
      }
  }
    if (TDOG>0) { 
      TDOG--;
      if (TDOG==0){
         setWord(&TSW,FDOG,true);
      }
  }
  interrupts();
  
}

//***********************************************************************************************************************************************************
//*--------------------------------------------------------------------------------------------
//* Interrupt vector PCINT0  (manages interrupts for D8 to D13, used only for D13
//*--------------------------------------------------------------------------------------------
ISR (PCINT0_vect) 
{
   //managePTT();    
   interrupts();
   
}
//*--------------------------------------------------------------------------------------------
ISR (PCINT1_vect) 
{
    // manage interrupts PCINT A0 to A5
    managePTT();
    interrupts();    

}  
//*--------------------------------------------------------------------------------------------
ISR (PCINT2_vect) 
{
  Encoder_san();
  interrupts();
   
}  

