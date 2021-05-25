//=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
//*                                              DSP Processing
//=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=

//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//*                       ADC Control
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
#define ADCSRA_MASK    0B00000110    //* Controls quality of audio sampling (250 KHz)
//#define ADCSRA_MASK    0B00000111    //* Controls quality of audio sampling (125 KHz)

#define TIMER_SAMPLERATE       51    //* Controls number of samples
#define NOTVI                   1    //* Defines no amplitude support (only phase)

#define F_SAMP_TX 4810        //4810 // ADC sample-rate;
#define _UA  (F_SAMP_TX)      //360  // 


static float t = 0;
static float prev_f = 0;
static float prev_phase = 0;
static float acc = 0;


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

/*
//*------------------------------------------------------------------------------------------
//* filter
//* Hilbert filter implementation
//*------------------------------------------------------------------------------------------
void filter(int val, int* i, int* q)
{
    int j;
    for (j = 0; j < 44; j++) {
        xv[j] = xv[j+1];
    }
    xv[44] = val;

    *i = xv[22];

    int _q = (xv[01] +     xv[3] +     xv[5] +     xv[7]+       xv[7] + 4*xv[9] + 6*xv[11] \
          + 9*xv[13] + 14*xv[15] + 23*xv[17] + 41*xv[19] + 127*xv[21]    \
        -(127*xv[23] + 41*xv[25] + 23*xv[27] + 14*xv[29] +   9*xv[31]   \
        +   6*xv[33] +  4*xv[35] +    xv[37]+     xv[37] +     xv[39] +  xv[41] + xv[43]) ) / 202;

  *q = _q;
}
//*------------------------------------------------------------------------------------------
//* arctan2   (TO BE REPLACED BY A FASTER ONE)
//* Inverse Tangent function
//*------------------------------------------------------------------------------------------
int arctan2(int y, int x)
{
   int abs_y = abs(y);
   int angle;
   if(x >= 0){
      angle = 45 - 45 * (x - abs_y) / ((x + abs_y)==0?1:(x + abs_y));
   } else {
      angle = 135 - 45 * (x + abs_y) / ((abs_y - x)==0?1:(abs_y - x));
   }
   return (y < 0) ? -angle : angle; // negate if in quad III or IV
}


//*-----------------------------------------------------------------------------------------------
//* ssb
//* ssb generator
//*-----------------------------------------------------------------------------------------------
void ssb(float in, float fsamp, float* amp, float* df)
{
   int i, q;
   float phase;
   //char str_temp[6];

   
   //t++;
   //digitalWrite(pinTEST,1);
   filter(in * 128, &i, &q);
   //digitalWrite(pinTEST,0);
   
//*------ Amplitude information isn't computed this firmware is aimed to FT8, some precious time is saved
   *amp=1.0;    //DEBUG only
   
#ifndef NOTVI

   *amp = sqrt( i*i + q*q) / 128.0f; 
   if(*amp > 1.0){
     printf("amp overflow %f\n", *amp);
     *amp = 1.0;
   }
    
#else
   *amp=1.0;   
#endif

   
   phase = M_PI + ((float)arctan2(q,i)) * M_PI/180.0f;
   float dp = phase - prev_phase;
   if(dp < 0) dp = dp + 2*M_PI;
   prev_phase = phase;

   *df = dp*fsamp/(2.0f*M_PI);
   
}
*/

//*=====
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
//*======
#define magn(i, q) (abs(i) > abs(q) ? abs(i) + abs(q) / 4 : abs(q) + abs(i) / 4) // approximation of: magnitude = sqrt(i*i + q*q); error 0.95dB

//uint8_t lut[256];
volatile uint8_t amp;
volatile uint8_t vox_thresh = (1 << 2);
volatile uint8_t drive = 2;   // hmm.. drive>2 impacts cpu load..why?

//*======
inline int16_t ssb(int16_t in)
{
  static int16_t dc;

  int16_t i, q;
  uint8_t j;
  static int16_t v[16];

  for(j = 0; j != 15; j++) v[j] = v[j + 1];

  dc += (in - dc) / 2;
  v[15] = in - dc;     // DC decoupling
  //dc = in;  // this is actually creating a high-pass (emphasis) filter

  i = v[7];
  q = ((v[0] - v[14]) * 2 + (v[2] - v[12]) * 8 + (v[4] - v[10]) * 21 + (v[6] - v[8]) * 15) / 128 + (v[6] - v[8]) / 2; // Hilbert transform, 40dB side-band rejection in 400..1900Hz (@4kSPS) when used in image-rejection scenario; (Hilbert transform require 5 additional bits)

  //uint16_t _amp = magn(i, q);
  
  //if(vox) _vox(_amp > vox_thresh);
  //_amp = (_amp > vox_thresh) ? _amp : 0;   // vox_thresh = 1 is a good setting
  //_amp = _amp << (drive);
  
#ifdef CONSTANT_AMP
//  if(_amp < 4 ){ amp = 0; return 0; } //hack: for constant amplitude cases, set drive=1 for good results
//  //digitalWrite(RX, (_amp < 4)); // fast on-off switching for constant amplitude case
#endif

  //amp=0;
  
  //_amp = ((_amp > 255) || (drive == 8)) ? 255 : _amp; // clip or when drive=8 use max output
  //amp = (tx) ? lut[_amp] : 0;

  static int16_t prev_phase;
  int16_t phase = arctan3(q, i);

  int16_t dp = phase - prev_phase;  // phase difference and restriction
  //dp = (amp) ? dp : 0;  // dp = 0 when amp = 0
  prev_phase = phase;

  if(dp < 0) dp = dp + _UA; // make negative phase shifts positive: prevents negative frequencies and will reduce spurs on other sideband

//#ifdef MAX_DP
//  if(dp > MAX_DP){ // dp should be less than half unit-angle in order to keep frequencies below F_SAMP_TX/2
//    prev_phase = phase - (dp - MAX_DP);  // substract restdp
//    dp = MAX_DP;
//  }
//#endif

  if(MODE == MUSB)
    return dp * ( F_SAMP_TX / _UA); // calculate frequency-difference based on phase-difference
  else
    return dp * (-F_SAMP_TX / _UA);
}

//*----------------------------------------------------------------------------------------------------
//* generator
//* main SSB generator
//*----------------------------------------------------------------------------------------------------
void generator(){

//float df, amp;
  if (MODE!=MLSB && MODE !=MUSB) { return; }

 
//  long int lap=micros();
  int16_t data = queueADC[0];     
  //ssb(data);
  int16_t df = ssb(data); // convert analog input into phase-shifts (carrier out by periodic frequency shifts)

#ifndef NOTVI
/*
float A = 87.7f; // compression parameter
      amp = (fabs(amp) < 1.0f/A) ? A*fabs(amp)/(1.0f+ln(A)) : (1.0f+ln(A*fabs(amp)))/(1.0f+ln(A)); //compand

  int ampval = (int)(round(amp * 8.0f)) - 1;
  int enaval = (ampval < 0) ? 0 : 1;

      if(notvi && ampval < 0){
         enaval = 1; // tx always on
         df = 0;
      }
      if(notvi) ampval = 7; // optional: no amplitude changes

      if(ampval>7) ampval=7;
      if(ampval<0) ampval=0;
*/
#endif

      // here, send ampval to 3 bit drive-strength register (if available)
      // here, add or substract df from PLL center frequency (in Hz), adding will make USB, substract. LSB

      byte v=(getWord(FT817,VFO)==false ? VFOA : VFOB);

      digitalWrite(pinTEST,1);
      if (MODE==MUSB) {
         SendFrequency(vfo[v]+df);
      } else {
         SendFrequency(vfo[v]+df);
      }
      digitalWrite(pinTEST,0);
 
}
