//=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
//*                                              SDR Processing

//=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
//*==============================================================================================================================================================
//* Code excerpt from QCX-SSB firmware by Guido (PE1NNZ)
//* QCX-SSB.ino - https://github.com/threeme3/QCX-SSB
//*
//* Copyright 2019, 2020   Guido PE1NNZ <pe1nnz@amsat.org>
//
//  with minor adaptations and modification from 
//  Pixie-SSB.ino http://github.com/lu7did/Pixie-SSB
//
//  Copyright 2021 Pedro LU7DID <lu7did@gmail.com>
//*==============================================================================================================================================================
#undef F_CPU
//#define F_CPU 20007000   // myqcx1:20008440, myqcx2:20006000   // Actual crystal frequency of 20MHz XTAL1, note that this declaration is just informative and does not correct the timing in Arduino functions like delay(); hence a 1.25 factor needs to be added for correction.
#define F_CPU 16000000   // myqcx1:20008440, myqcx2:20006000   // Actual crystal frequency of 20MHz XTAL1, note that this declaration is just informative and does not correct the timing in Arduino functions like delay(); hence a 1.25 factor needs to be added for correction.

#define DEBUG  1   // enable testing and diagnostics features
//enum mode_t { LSB, USB, CW, AM, FM };    //* REVISAR se necesita integrar ésta codificación para los modos con la utilizada por el objeto CAT

volatile int8_t   mode = MUSB;
volatile uint16_t numSamples = 0;
volatile uint8_t  tx = 0;
volatile bool     vox = false;
//--------------------------------------------------------------------------------------------------
// _vox
// Manage VOX activation
//--------------------------------------------------------------------------------------------------
inline void _vox(uint8_t trigger)
{
  if(trigger){
    
    
    if(!tx){
    /*  
      TX can be enabled here 
    
    */
    
    }
    tx = (vox) ? 255 : 1; // hangtime = 255 / 4402 = 58ms (the time that TX at least stays on when not triggered again)
  } else {
    if(tx){
      tx--;
      
        
       
      if(!tx){ 
        /* 
        
        RX can be enabled here 
      
        */
      
      }
    }
  }
}

#define F_SAMP_TX 4810        //4810 // ADC sample-rate; is best a multiple of _UA and fits exactly in OCR0A = ((F_CPU / 64) / F_SAMP_TX) - 1 , should not exceed CPU utilization
#define _UA  (F_SAMP_TX)      //360  // unit angle; integer representation of one full circle turn or 2pi radials or 360 degrees, should be a integer divider of F_SAMP_TX and maximized to have higest precision

//#define MAX_DP  (_UA/1)  //(_UA/2) // the occupied SSB bandwidth can be further reduced by restricting the maximum phase change (set MAX_DP to _UA/2).
//#define CONSTANT_AMP  1 // enable this in case there is no circuitry for controlling envelope (key shaping circuit)
//#define CARRIER_COMPLETELY_OFF_ON_LOW  1    // disable oscillator on no-envelope transitions, to prevent potential unwanted biasing/leakage through PA circuit
//#define MULTI_ADC  1  // multiple ADC conversions for more sensitive (+12dB) microphone input

//--------------------------------------------------------------------------------------------------
// DSP processing functions
//--------------------------------------------------------------------------------------------------

/*
 * arctan3(x)
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

/*
 * magn(i,q)
 */ 

#define magn(i, q) (abs(i) > abs(q) ? abs(i) + abs(q) / 4 : abs(q) + abs(i) / 4) // approximation of: magnitude = sqrt(i*i + q*q); error 0.95dB

uint8_t          lut[256];
volatile uint8_t amp;
volatile uint8_t vox_thresh = (1 << 2);
volatile uint8_t drive = 2;   // hmm.. drive>2 impacts cpu load..why?

/*
 * process audio sample signal into an I/Q SSB signal
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

  int16_t dp = phase - prev_phase;  // phase difference and restriction
  prev_phase = phase;

  if(dp < 0) dp = dp + _UA; // make negative phase shifts positive: prevents negative frequencies and will reduce spurs on other sideband

#ifdef MAX_DP
  if(dp > MAX_DP){ // dp should be less than half unit-angle in order to keep frequencies below F_SAMP_TX/2
    prev_phase = phase - (dp - MAX_DP);  // substract restdp
    dp = MAX_DP;
  }
#endif

  if(mode == MUSB)
    return dp * ( F_SAMP_TX / _UA); // calculate frequency-difference based on phase-difference
  else
    return dp * (-F_SAMP_TX / _UA);
}


#define MIC_ATTEN  0  // 0*6dB attenuation (note that the LSB bits are quite noisy)
volatile int8_t mox = 0;
volatile int8_t volume = 12;

/*

 This is the ADC ISR, issued with sample-rate via timer1 compb interrupt.
 It performs in real-time the ADC sampling, calculation of SSB phase-differences, calculation of SI5351 frequency registers and send the registers to SI5351 over I2C.

*/ 

static int16_t _adc;
void dsp_tx()
{
  // jitter dependent things first
  
#ifdef MULTI_ADC  // SSB with multiple ADC conversions:

  int16_t adc;                         // current ADC sample 10-bits analog input, NOTE: first ADCL, then ADCH
  adc = ADC;
  ADCSRA |= (1 << ADSC);
  //OCR1BL = amp;                        // submit amplitude to PWM register (actually this is done in advance (about 140us) of phase-change, so that phase-delays in key-shaping circuit filter can settle)
  si5351.SendPLLBRegisterBulk();       // submit frequency registers to SI5351 over 731kbit/s I2C (transfer takes 64/731 = 88us, then PLL-loopfilter probably needs 50us to stabalize)
  OCR1BL = amp;                      // submit amplitude to PWM register (takes about 1/32125 = 31us+/-31us to propagate) -> amplitude-phase-alignment error is about 30-50us
  adc += ADC;
  //ADCSRA |= (1 << ADSC);  // causes RFI on QCX-SSB units (not on units with direct biasing); ENABLE this line when using direct biasing!!

  
  int16_t df = ssb(_adc >> MIC_ATTEN); // convert analog input into phase-shifts (carrier out by periodic frequency shifts)
  adc += ADC;
  ADCSRA |= (1 << ADSC);
  si5351.freq_calc_fast(df);           // calculate SI5351 registers based on frequency shift and carrier frequency
  adc += ADC;
  ADCSRA |= (1 << ADSC);
  _adc = (adc/4 - 512);

#else  // SSB with single ADC conversion:

  /*
   * SSB Generation SDR segment (this is the mero mero honcho stuff)
   */


  ADCSRA |= (1 << ADSC);    // start next ADC conversion (trigger ADC interrupt if ADIE flag is set)
  //si5351.SendPLLBRegisterBulk();       // submit frequency registers to SI5351 over 731kbit/s I2C (transfer takes 64/731 = 88us, then PLL-loopfilter probably needs 50us to stabalize)
  OCR1BL = amp;                        // submit amplitude to PWM register (takes about 1/32125 = 31us+/-31us to propagate) -> amplitude-phase-alignment error is about 30-50us
  int16_t adc = ADC - 512; // current ADC sample 10-bits analog input, NOTE: first ADCL, then ADCH
  int16_t df = ssb(adc >> MIC_ATTEN);  // convert analog input into phase-shifts (carrier out by periodic frequency shifts)
  //si5351.freq_calc_fast(df);           // calculate SI5351 registers based on frequency shift and carrier frequency
  si5351.set_freq((df * 100ULL), SI5351_CLK2);

#endif

#ifdef CARRIER_COMPLETELY_OFF_ON_LOW
  if(OCR1BL == 0){ si5351.SendRegister(SI_CLK_OE, (amp) ? 0b11111011 : 0b11111111); } // experimental carrier-off for low amplitudes
#endif

  if(!mox) return;
  OCR1AL = (adc << (mox-1)) + 128;  // TX audio monitoring
}

