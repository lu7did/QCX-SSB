#include <Wire.h>
#include <SPI.h>
#include <LiquidCrystal.h>
#include <si5351.h>

const uint32_t bandStart = 7000000;     // start of 40m
const uint32_t bandEnd =   7300000;     // end of 40m
const uint32_t bandInit =  7100000;     // where to initially set the frequency
volatile long oldfreq = 0;
volatile long currentfreq = 0;
volatile int updatedisplay = 0;

volatile uint32_t freq = bandInit ;     // this is a variable (changes) - set it to the beginning of the band
volatile uint32_t radix = 1000;         // how much to change the frequency by, clicking the rotary encoder will change this.
volatile uint32_t oldradix = 1;

volatile uint32_t BFO_freq = 8003000;

// Rotary encoder pins and other inputs
static const int pushPin = 9;
static const int rotBPin = 2;
static const int rotAPin = 3;

// Rotary encoder variables, used by interrupt routines
volatile int rotState = 0;
volatile int rotAval = 1;
volatile int rotBval = 1;

// Instantiate the Objects
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
Si5351 si5351;


int adc_key_in  = 0;

#define btnRIGHT  0
#define btnUP     1 
#define btnDOWN   2 
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5   
#define btnEncodeOK  6 
void setup()
{
  // Set up frequency and radix switches
  pinMode(rotAPin, INPUT);
  pinMode(rotBPin, INPUT);
  pinMode(pushPin, INPUT);


  pinMode(10,OUTPUT);
  digitalWrite(10,HIGH);

  // Set up pull-up resistors on inputs
  digitalWrite(rotAPin, HIGH);
  digitalWrite(rotBPin, HIGH);
  digitalWrite(pushPin, HIGH);

  // Set up interrupt pins
  attachInterrupt(digitalPinToInterrupt(rotAPin), ISRrotAChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(rotBPin), ISRrotBChange, CHANGE);

  // Initialize the display
  lcd.begin(16,2);
  //lcd.backlight();
  
  UpdateDisplay();
  delay(1000);

  // Initialize the DDS
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  si5351.set_correction(31830, SI5351_PLL_INPUT_XO);      // Set to specific Si5351 calibration number
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
    si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_8MA);
  si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_8MA);
  si5351.set_freq((freq * 100ULL), SI5351_CLK0);
  si5351.set_freq((freq * 100ULL), SI5351_CLK1);
  si5351.set_freq((BFO_freq * 100ULL), SI5351_CLK2);
}


void loop()
{

  
  currentfreq = getfreq();                // Interrupt safe method to get the current frequency

  if (currentfreq != oldfreq)
  {
    UpdateDisplay();
    SendFrequency();
    oldfreq = currentfreq;
  }

  
  if (digitalRead(pushPin) == LOW)
  {
    delay(10);
    while (digitalRead(pushPin) == LOW)
    {
      if (updatedisplay == 1)
      {
        UpdateDisplay();
        updatedisplay = 0;
      }
    }
    delay(50);
  }
}


long getfreq()
{
  long temp_freq;
  cli();
  temp_freq = freq;
  sei();
  return temp_freq;
}


// Interrupt routines
void ISRrotAChange()
{
  if (digitalRead(rotAPin))
  {
    rotAval = 1;
    UpdateRot();
  }
  else
  {
    rotAval = 0;
    UpdateRot();
  }
}


void ISRrotBChange()
{
  if (digitalRead(rotBPin))
  {
    rotBval = 1;
    UpdateRot();
  }
  else
  {
    rotBval = 0;
    UpdateRot();
  }
}


void UpdateRot()
{
  switch (rotState)
  {

    case 0:                                         // Idle state, look for direction
      if (!rotBval)
        rotState = 1;                               // CW 1
      if (!rotAval)
        rotState = 11;                              // CCW 1
      break;

    case 1:                                         // CW, wait for A low while B is low
      if (!rotBval)
      {
        if (!rotAval)
        {
          freq = freq + radix;
          if (freq > bandEnd) {
              freq = bandEnd;
          }    
          SendFrequency();
          UpdateDisplay();
          rotState = 2;                             // CW 2
        }
      }
      else if (rotAval)
        rotState = 0;                             // It was just a glitch on B, go back to start
      break;

    case 2:                                         // CW, wait for B high
      if (rotBval)
        rotState = 3;                               // CW 3
      break;

    case 3:                                         // CW, wait for A high
      if (rotAval)
        rotState = 0;                               // back to idle (detent) state
      break;

    case 11:                                        // CCW, wait for B low while A is low
      if (!rotAval)
      {
        if (!rotBval)
        {
          freq = freq - radix;
          if (freq < bandStart) {
              freq = bandStart;
          }    
          SendFrequency();
          UpdateDisplay();         
          rotState = 12;                            // CCW 2
        }
      }
      else if (rotBval)
        rotState = 0;                             // It was just a glitch on A, go back to start
      break;

    case 12:                                        // CCW, wait for A high
      if (rotAval)
        rotState = 13;                              // CCW 3
      break;

    case 13:                                        // CCW, wait for B high
      if (rotBval)
        rotState = 0;                               // back to idle (detent) state
      break;
  }
}


void UpdateDisplay()
{
  lcd.setCursor(0, 0);
  lcd.setCursor(0, 0);
  lcd.print(freq);
  lcd.setCursor(0, 1);
  lcd.print("ZL2CTM");

  if (radix != oldradix)                          // stops radix display flashing/blinking on freq change
  {
    lcd.setCursor(9, 0);
    lcd.print("       ");
    lcd.setCursor(9, 0);
    if (radix == 1)
      lcd.print("   1 Hz");
    if (radix == 10)
      lcd.print("  10 Hz");
    if (radix == 100)
      lcd.print(" 100 Hz");
    if (radix == 1000)
      lcd.print("  1 kHz");
    if (radix == 10000)
      lcd.print(" 10 kHz");
    if (radix == 100000)
      lcd.print("100 kHz");
    oldradix = radix;
  }
}


void SendFrequency()
{
  si5351.set_freq((freq * 100ULL), SI5351_CLK2);
  //si5351.set_freq((BFO_freq * 100ULL), SI5351_CLK0);
}




