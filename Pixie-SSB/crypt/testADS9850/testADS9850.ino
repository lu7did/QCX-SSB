#include <AD9850.h>
#include <Wire.h>

const int W_CLK_PIN = 11;
const int FQ_UD_PIN = 12;
const int DATA_PIN = 13;
const int RESET_PIN = A0;

double freq = 7030000;
double trimFreq = 124999500;

int phase = 0;
char hi[80];
void printDouble( double val, unsigned int precision){
// prints val with number of decimal places determine by precision
// NOTE: precision is 1 followed by the number of zeros for the desired number of decimial places
// example: printDouble( 3.1415, 100); // prints 3.14 (two decimal places)

    double v=val/1000.;
    
    Serial.print (int(v));  //prints the int part
    Serial.print("."); // print the decimal point
    unsigned int frac;
    if(val >= 0)
        frac = (v - int(v)) * precision;
    else
        frac = (int(v)- v ) * precision;
    Serial.println(frac,DEC) ;
}
void setup(){
  DDS.begin(W_CLK_PIN, FQ_UD_PIN, DATA_PIN, RESET_PIN);
  //DDS.calibrate(trimFreq);
  Serial.begin(9600);
  DDS.up();
  
}
void pulsePin(int pin) {
  digitalWrite(pin, HIGH);
  digitalWrite(pin, LOW);
}

void loop(){
  //DDS.setfreq(freq, phase);
  //printDouble(freq,10);
  //sprintf(hi,"phase=%d\n",phase);
  //Serial.println(hi);
  //delay(10000);
  //DDS.down();
  //delay(3000);
  //DDS.up();
  //delay(2000);
  //DDS.setfreq(freq + 500, phase);
  //delay(5000);
  //DDS.down();
  DDS.setfreq(7030000,0);

  while(1) {
    //DDS.up();
    //pulsePin(W_CLK_PIN);
    //pulsePin(FQ_UD_PIN);
    //pulsePin(DATA_PIN);
    //pulsePin(RESET_PIN);
  }
}

