/*
 * testADC.ino
 * This is a test component of the Pixie-SSB project to evaluate the best strategy
 * to sample and process the audio input as well as the management of levels to be used
 * Some excerpts to be incorporated as a callibration tool (ala QCX).
 * 
 * based on a sketch found at http://gammon.com.au/adc by Nick Gammon
 * 
 */

#define VERSION "1.0"
#define PROGRAMID "testADC"

#include <avr/sleep.h>
#include <avr/wdt.h>
#include <LiquidCrystal.h>
#include <AD9850.h>
#include <asserts.h>
#include <errors.h>

#define ROT_A   2         //PD6    (pin 12)
#define ROT_B   3         //PD7    (pin 13)
#define LCD_D4  4         //PD0    (pin 2)
#define LCD_D5  5         //PD1    (pin 3)
#define LCD_D6  6         //PD2    (pin 4)
#define LCD_D7  7         //PD3    (pin 5)
#define LCD_RS  8         //PC4    (pin 27)
#define LCD_EN  9         //PD4    (pin 6)
#define LCD_BL  10        //LCD Backlight enabler



const byte adcPin = 2;  // A2
char hi[80];
const int MAX_RESULTS = 256;

volatile int results [MAX_RESULTS];
volatile unsigned int stamp[MAX_RESULTS];
volatile int resultNumber =0;
volatile int vmin=1023;
volatile int vmax=0;

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);  // LCD handling system     
int32_t f=7030000;

volatile int8_t encoder_val = 0;
volatile int8_t encoder_step = 0;
static uint8_t last_state;

/*
 * Interrupt handler for ROT_A/ROT_B pin change
 */

ISR(PCINT2_vect){  // Interrupt on rotary encoder turn
  switch(last_state = (last_state << 4) | (digitalRead(ROT_B) << 1) | digitalRead(ROT_A)){ //transition  (see: https://www.allaboutcircuits.com/projects/how-to-use-a-rotary-encoder-in-a-mcu-based-project/  )
    case 0x31: case 0x10: case 0x02: case 0x23: if(encoder_step < 0) encoder_step = 0; encoder_step++; if(encoder_step >  3){ encoder_step = 0; encoder_val++; } break;
    case 0x32: case 0x20: case 0x01: case 0x13: if(encoder_step > 0) encoder_step = 0; encoder_step--; if(encoder_step < -3){ encoder_step = 0; encoder_val--; } break;  
  }
}
/*
 * Configuration for Encoder
 */
void encoder_setup()
{
  pinMode(ROT_A, INPUT_PULLUP);
  pinMode(ROT_B, INPUT_PULLUP);
  PCMSK2 |= (1 << PCINT19) | (1 << PCINT18); // interrupt-enable for ROT_A, ROT_B pin changes; 
  PCICR |= (1 << PCIE2); 
  last_state = (digitalRead(ROT_B) << 1) | digitalRead(ROT_A);
  interrupts();
}
// ADC complete ISR
ISR (ADC_vect)
  {
  if (resultNumber >= MAX_RESULTS)
    ADCSRA = 0;  // turn off ADC
  else
    results [resultNumber] = ADC;
    stamp[resultNumber]=micros();  
    if (ADC>vmax) vmax=ADC;
    if (ADC<vmin) vmin=ADC; 
    resultNumber++;

  }  // end of ADC_vect
  
EMPTY_INTERRUPT (TIMER1_COMPB_vect);
 
void setup ()
  {
  Serial.begin (9600);
  sprintf(hi,"%s version %s\n",PROGRAMID,VERSION);
  Serial.print(hi);
  Serial.println ();
  
  // reset Timer 1
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  TCCR1B = bit (CS11) | bit (WGM12);  // CTC, prescaler of 8
  TIMSK1 = bit (OCIE1B);  // WTF?
  OCR1A = 39;    
  OCR1B = 39;   // 20 uS - sampling frequency 50 kHz

  ADCSRA =  bit (ADEN) | bit (ADIE) | bit (ADIF);   // turn ADC on, want interrupt on completion
  ADCSRA |= bit (ADPS2);  // Prescaler of 16
  ADMUX = bit (REFS0) | (adcPin & 7);
  ADCSRB = bit (ADTS0) | bit (ADTS2);  // Timer/Counter1 Compare Match B
  ADCSRA |= bit (ADATE);   // turn on automatic triggering

  // wait for buffer to fill
  while (resultNumber < MAX_RESULTS)
    { }
  lcd.begin(16,2);
  digitalWrite(LCD_BL,1);
  lcd.setCursor(0,0);
/*  
 *   setup encoder
 */
  encoder_setup();
/*  
 *   Initialize AD9850
 */
  lcd.print("testADC");
    
  for (int i = 0; i < MAX_RESULTS; i++) {
    sprintf(hi,"%d,%d,%d,%d\n",i,results[i],results[i]-((vmax+vmin)/2),stamp[i]);       
    Serial.print(hi);

  }
  int vm=(vmax+vmin)/2;
  float vmaxpk=((vmax-vm)*1.0/1023.0)*5.0;
  float vminpk=((vmin-vm)*1.0/1023.0)*5.0;
  float vpkpk=vmaxpk+abs(vminpk);

  sprintf(hi, "Vpk-pk:%d mV", int(vpkpk*1000));
  //sprintf(hi,"V pk-pk=%d V",int(vpkpk*1000));
  lcd.setCursor(0,1);
  lcd.print(hi);
  
  }  // end of setup

void loop () { }
