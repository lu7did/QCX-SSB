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
#define PROGRAMID "testSSB"

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
const int MAX_RESULTS = 128;

volatile int results [MAX_RESULTS];
//volatile unsigned int stamp[MAX_RESULTS];
volatile long int phase[MAX_RESULTS];
volatile int resultNumber =0;
volatile int vmin=1023;
volatile int vmax=0;
volatile int vm=0;
volatile bool trigger=false;
volatile int vant=0;
volatile int16_t p;
volatile long int cnt=0;
volatile unsigned int t0=0;
volatile int16_t t=0;
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

/*========================================================[DSP Core]======================================================================
 * Extracted from QCX-SSB
 =========================================================================================================================================*/
//#define F_SAMP_TX 4810        //4810 // ADC sample-rate; is best a multiple of _UA and fits exactly in OCR0A = ((F_CPU / 64) / F_SAMP_TX) - 1 , should not exceed CPU utilization
//#define _UA  (F_SAMP_TX)      //360  // unit angle; integer representation of one full circle turn or 2pi radials or 360 degrees, should be a integer divider of F_SAMP_TX and maximized to have higest precision

#define F_SAMP_TX 50000        //4810 // ADC sample-rate; is best a multiple of _UA and fits exactly in OCR0A = ((F_CPU / 64) / F_SAMP_TX) - 1 , should not exceed CPU utilization
#define _UA  (F_SAMP_TX)      //360  // unit angle; integer representation of one full circle turn or 2pi radials or 360 degrees, should be a integer divider of F_SAMP_TX and maximized to have higest precision

#undef  MULTI_ADC 
//#define CONSTANT_AMP  1  //Constant amplitude, Pixie brings a Class C amplifier which is not able to manage amplitude, FT8 is a constant amplitude mode though
#undef CONSTANT_AMP
volatile uint8_t tx          = 255; //directly on transmit mode
volatile bool vox            = false;
enum mode_t { LSB, USB, CW, AM, FM };

/*
 * Status variable areas
 */
 
volatile int8_t mode         = USB;

inline void _vox(uint8_t trigger)
{
  if(trigger){
    //if(!tx){ /* TX can be enabled here */ }
    tx = (vox) ? 255 : 1; // hangtime = 255 / 4402 = 58ms (the time that TX at least stays on when not triggered again)
  } else {
    if(tx){
      tx--;
      //if(!tx){ /* RX can be enabled here */ }
    }
  }
}


/*
 * arctan3
 * Fast Arc Tg(x) implementation
 */
inline int16_t arctan3(int16_t q, int16_t i)  // error ~ 0.8 degree
{ // source: [1] http://www-labs.iro.umontreal.ca/~mignotte/IFT2425/Documents/EfficientApproximationArctgFunction.pdf
#define _atan2(z)  (_UA/8 - _UA/22 * z + _UA/22) * z  //derived from (5) [1]
  //#define _atan2(z)  (_UA/8 - _UA/24 * z + _UA/24) * z  //derived from (7) [1]
  int16_t r;
  if(abs(q) > abs(i))
    r = _UA / 4 - _atan2(abs(i) / abs(q));        // arctan(z) = 90-arctan(1/z)
  else
    r = (i == 0) ? 0 : _atan2(abs(q) / abs(i));   // arctan(z)
  r = (i < 0) ? _UA / 2 - r : r;                  // arctan(-z) = -arctan(z)
  return (q < 0) ? -r : r;                        // arctan(-z) = -arctan(z)
}

#define magn(i, q) (abs(i) > abs(q) ? abs(i) + abs(q) / 4 : abs(q) + abs(i) / 4) // approximation of: magnitude = sqrt(i*i + q*q); error 0.95dB

uint8_t lut[256];
volatile uint8_t amp;
volatile uint8_t vox_thresh = (1 << 2);
volatile uint8_t drive = 2;   // hmm.. drive>2 impacts cpu load..why?

/*
 * ssb processor
 * given an audio sample of 16 bits process the generation of SSB
 * by means of creating the I/Q signal pair
 */
inline int16_t ssb(int16_t in)
{
  static int16_t dc;

  int16_t i, q;
  uint8_t j;
  static int16_t v[16];

  for(j = 0; j != 15; j++) v[j] = v[j + 1];

  dc += (in - dc) / 2;
  v[15] = in - dc;     // DC decoupling

  i = v[7];
  q = ((v[0] - v[14]) * 2 + (v[2] - v[12]) * 8 + (v[4] - v[10]) * 21 + (v[6] - v[8]) * 15) / 128 + (v[6] - v[8]) / 2; // Hilbert transform, 40dB side-band rejection in 400..1900Hz (@4kSPS) when used in image-rejection scenario; (Hilbert transform require 5 additional bits)

  uint16_t _amp = magn(i, q);
  if(vox) _vox(_amp > vox_thresh);

  _amp = _amp << (drive);

#ifdef CONSTANT_AMP
  if(_amp < 4 ){ amp = 0; return 0; } //hack: for constant amplitude cases, set drive=1 for good results
  
#endif
  _amp = ((_amp > 255) || (drive == 8)) ? 255 : _amp; // clip or when drive=8 use max output
  amp = (tx) ? lut[_amp] : 0;

  static int16_t prev_phase;
  int16_t phase = arctan3(q, i);
  //return phase;

  int16_t dp = phase - prev_phase;  // phase difference and restriction
  prev_phase = phase;

  if(dp < 0) dp = dp + _UA; // make negative phase shifts positive: prevents negative frequencies and will reduce spurs on other sideband

#ifdef MAX_DP
  if(dp > MAX_DP){ // dp should be less than half unit-angle in order to keep frequencies below F_SAMP_TX/2
    prev_phase = phase - (dp - MAX_DP);  // substract restdp
    dp = MAX_DP;
  }
#endif

  if(mode == USB)
    return dp * ( F_SAMP_TX / _UA); // calculate frequency-difference based on phase-difference
  else
    return dp * (-F_SAMP_TX / _UA);
}

/*
 * Manage audio input
 */
 
#define MIC_ATTEN  0  // 0*6dB attenuation (note that the LSB bits are quite noisy)
volatile int8_t mox = 0;
volatile int8_t volume = 12;

// This is the ADC ISR, issued with sample-rate via timer1 compb interrupt.
// It performs in real-time the ADC sampling, calculation of SSB phase-differences, calculation of SI5351 frequency registers and send the registers to SI5351 over I2C.

static int16_t _adc;

/*
 * This is the core DSP-SDR processing where the signal is generated
 */
void dsp_tx() {
// jitter dependent things first


#undef MULTI_ADC

//*-----------------------------------------------------------------------------------------------------------------------------
#ifdef MULTI_ADC  // SSB with multiple ADC conversions:
  int16_t adc;                         // current ADC sample 10-bits analog input, NOTE: first ADCL, then ADCH
  
  adc = ADC;
  ADCSRA |= (1 << ADSC);
  int16_t df = ssb(_adc >> MIC_ATTEN); // convert analog input into phase-shifts (carrier out by periodic frequency shifts)
  adc += ADC;
  ADCSRA |= (1 << ADSC);
  si5351.freq_calc_fast(df);           // calculate SI5351 registers based on frequency shift and carrier frequency
  adc += ADC;
  ADCSRA |= (1 << ADSC);
  _adc = (adc/4 - 512);
#else  // SSB with single ADC conversion which is the one PIXIE uses
  if (!(ADCSRA & bit(ADIF))) {
    return;
  }
  ADCSRA |= (1 << ADSC);
  int16_t adc = ADC - 512; // current ADC sample 10-bits analog input, NOTE: first ADCL, then ADCH
  int16_t df = ssb(adc >> MIC_ATTEN);  // convert analog input into phase-shifts (carrier out by periodic frequency shifts)
#endif

}

/*
 * NO UTILIZADO
 */

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
    //results[resultNumber] = ADC;
    //stamp[resultNumber]=micros();  
    if (ADC>vmax) {
       vmax=ADC;
       vm=(vmax+vmin)/2; 
       t=vmax/2;
    }
    if (ADC<vmin) {
       vmin=ADC;
       vm=(vmax+vmin)/2;
       t=vmax/2;
    }
    results[resultNumber] = ADC-vm;

    if (resultNumber >0 && (results[resultNumber]>results[resultNumber-1]) && trigger==false) {
       phase[resultNumber]=cnt;
       trigger=true;
       cnt=1;
    } else {
      cnt++;
    }

    if (results[resultNumber]<0) {
       trigger=false;
    }

   
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
  lcd.print("testSSB2");
  int lastf=0;
  int f=0;  
  for (int i = 0; i < MAX_RESULTS; i++) {
    if (phase[i]!=0) {
       f=50000/phase[i];
       sprintf(hi,"%d,%d,%ld    === f=%d\n",i,results[i],phase[i],f);       

    } else {
       sprintf(hi,"%d,%d,%ld\n",i,results[i],phase[i]);       
    }
    Serial.print(hi);

  }

  
  }  // end of setup

void loop () { }
