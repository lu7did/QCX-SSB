/*
 * testAD9850 
 * Sketch to test the LCD + encoder + AD9850 configuration for Pixie
 * 
 */
#define ROT_A   2         //PD6    (pin 12)
#define ROT_B   3         //PD7    (pin 13)
#define LCD_D4  4         //PD0    (pin 2)
#define LCD_D5  5         //PD1    (pin 3)
#define LCD_D6  6         //PD2    (pin 4)
#define LCD_D7  7         //PD3    (pin 5)
#define LCD_RS  8         //PC4    (pin 27)
#define LCD_EN  9         //PD4    (pin 6)
#define LCD_BL  10        //LCD Backlight enabler


#include <avr/sleep.h>
#include <avr/wdt.h>
#include "AD9850Lib.h"
#include <LiquidCrystal.h>
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
/*
 * firmware setup
 * 
 */
void setup() {

/* Init serial and LCD  
 *  
 */
  Serial.begin(9600);
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
  lcd.print("AD9850 Test");
  initDDS();
  Serial.print("DDS configuration completed\n");
  setDDS(f);
  
  sprintf(hi,"f=%ld",f);
  lcd.setCursor(0,1);
  lcd.print(hi);
  Serial.println(hi);
}

void loop() {
  // put your main code here, to run repeatedly:

      if(encoder_val){  // process encoder tuning steps

        sprintf(hi,"Encoder event(%d)\n",encoder_val);
        Serial.print(hi);
        f=f+(500*encoder_val);
        setDDS(f);
        sprintf(hi,"f=%ld",f);
        lcd.setCursor(0,1);
        lcd.print(hi);
        Serial.println(hi);
                
        encoder_val = 0;
    }

}
