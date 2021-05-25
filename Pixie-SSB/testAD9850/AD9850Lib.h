#include <asserts.h>
#include <errors.h>

//=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
//*                                              Si5351 Controller
//=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
#include <AD9850.h>
const int W_CLK_PIN = 11;
const int FQ_UD_PIN = 12;
const int DATA_PIN  = 13;
const int RESET_PIN = A1;

double trimFreq = 124999500;
int    phase = 0;
char   hi[80];

void initDDS() {

  DDS.begin(W_CLK_PIN, FQ_UD_PIN, DATA_PIN, RESET_PIN);
  DDS.calibrate(trimFreq);
  sprintf(hi,"AD9850 DDS initialized Ok\n");
  Serial.print(hi);
    
}

void setDDS(int32_t f) {

   double freq=f*1.0;
   DDS.setfreq(freq, phase);
   sprintf(hi,"AD9850 f=%ld\n",f);
   Serial.print(hi); 

}

