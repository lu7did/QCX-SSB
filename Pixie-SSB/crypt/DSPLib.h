//=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
//*                                              DSP Processing
//=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=

//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//*                       ADC Control
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//#define ADCSRA_MASK    0B00000110    //* Controls quality of audio sampling (250 KHz)
#define ADCSRA_MASK    0B00000111    //* Controls quality of audio sampling (125 KHz)

#define TIMER_SAMPLERATE       51    //* Controls number of samples
#define NOTVI                   1    //* Defines no amplitude support (only phase)

#define F_SAMP_TX 4810        //4810 // ADC sample-rate;
#define _UA  (F_SAMP_TX)      //360  // 


static float t = 0;
static float prev_f = 0;
static float prev_phase = 0;
static float acc = 0;

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


void sampleAudio(uint8_t pin)
{
      uint8_t low, high;
    uint8_t analog_reference = DEFAULT;
   long int lap=micros();
 
    if (pin >= 14) pin -= 14; // allow for channel or pin numbers
 
// set the analog reference (high two bits of ADMUX) and select the
// channel (low 4 bits).  this also sets ADLAR (left-adjust result)
// to 0 (the default).
    
    ADMUX = (analog_reference << 6) | (pin & 0x07);
 
// without a delay, we seem to read from the wrong channel
//delay(1);
 
// start the conversion

    //sbi(ADCSRA, ADSC);     //stablish 125 KHz sampling abt 100 uSec per conversion

//*EXPERIMENTAL* set sampling at 250 KHz or abt 50 uSec per conversion (13 steps to convert)
//*EXPERIMENTAL* this is controlled by the ADCSRA_MASK (now in 64, at 128 would be 0b00000111

    ADCSRA=(ADCSRA & 0b11111000) | ADCSRA_MASK; 
 
// ADSC is cleared when the conversion finishes

//    while (bit_is_set(ADCSRA, ADSC));
 
// we have to read ADCL first; doing so locks both ADCL
// and ADCH until ADCH is read.  reading ADCL second would
// cause the results of each conversion to be discarded,
// as ADCL and ADCH would be locked when it completed.

 //   low  = ADCL;
 //   high = ADCH;
 
// combine the two bytes

    //totTime[0]=totTime[0]+(micros()-lap);
    //DEBUG_adc=high<<8 | low;
//    return (high << 8) | low;
   
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//* sampleMIC
//* sample audio from A2  
//--------------------------------------------------------------------------------------------------------------------------------------------------------
int sampleMicrophone(uint8_t pin)
{
    uint8_t low, high;
    uint8_t analog_reference = DEFAULT;
   long int lap=micros();
 
    if (pin >= 14) pin -= 14; // allow for channel or pin numbers
 
// set the analog reference (high two bits of ADMUX) and select the
// channel (low 4 bits).  this also sets ADLAR (left-adjust result)
// to 0 (the default).
    
    ADMUX = (analog_reference << 6) | (pin & 0x07);
 
// without a delay, we seem to read from the wrong channel
//delay(1);
 
// start the conversion

    sbi(ADCSRA, ADSC);     //stablish 125 KHz sampling abt 100 uSec per conversion

//*EXPERIMENTAL* set sampling at 250 KHz or abt 50 uSec per conversion (13 steps to convert)
//*EXPERIMENTAL* this is controlled by the ADCSRA_MASK (now in 64, at 128 would be 0b00000111

    ADCSRA=(ADCSRA & 0b11111000) | ADCSRA_MASK; 
 
// ADSC is cleared when the conversion finishes

    while (bit_is_set(ADCSRA, ADSC));
 
// we have to read ADCL first; doing so locks both ADCL
// and ADCH until ADCH is read.  reading ADCL second would
// cause the results of each conversion to be discarded,
// as ADCL and ADCH would be locked when it completed.

    low  = ADCL;
    high = ADCH;
 
// combine the two bytes

    //totTime[0]=totTime[0]+(micros()-lap);
    //DEBUG_adc=high<<8 | low;
    return (high << 8) | low;
}

