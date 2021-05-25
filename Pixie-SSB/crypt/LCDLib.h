//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//*              CUSTOM LCD CHARACTER DEFINITIONS
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
/*
byte BB3[8] = {
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
};

 
byte BB5[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};
byte BB1[8] = {
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
};
byte BB2[8] = {
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
};
byte BB4[8] = {
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
};
*/

byte TX[8] = {
  B11111,
  B10001,
  B11011,
  B11011,
  B11011,
  B11011,
  B11111,
};
/*
byte SPLITM[8] = {
  B11111,
  B10001,
  B10111,
  B10001,
  B11101,
  B11101,
  B10001,
  B11111
};

byte RX[8] = {
  B11111,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B11111,
};

*/
byte MARK[8] = {
  B00010,
  B00110,
  B01110,
  B11110,
  B11110,
  B01110,
  B00110,
  B00010
};
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//*              LCD & ROTARY ENCODER SHIELD DEFINTIONS
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
#define btnRIGHT  0
#define btnUP     1 
#define btnDOWN   2 
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5   
#define btnEncodeOK  6 

#define LCD_ROWS  2
#define LCD_COLS 16

//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//*              GUI AND PRESENTATION CONTROL
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
/*
//*--------------------------------------------------------------------------------------------
//* showVFO
//* show VFO filter at the display
//*--------------------------------------------------------------------------------------------
void showVFO() {

  lcd.setCursor(0,0); lcd.print("A");
  lcd.setCursor(0,1); lcd.print("B");

}
*/
//*--------------------------------------------------------------------------------------------
//* showPTT
//* show PTT status (Transmit / Receive)
//*--------------------------------------------------------------------------------------------
void showPTT(){
  
  lcd.setCursor(13,0);
  if (getWord(FT817,PTT)==false) {
      lcd.write(byte(6));
  } else {
      lcd.print(" ");
  }
 
}
//*--------------------------------------------------------------------------------------------
//* showMode
//* show current Mode
//*--------------------------------------------------------------------------------------------
void showMode() {
char* m = " ";

   lcd.setCursor(13,1);
   switch(mode) {
     case MCW  : m="C"; break;
     case MUSB : m="U"; break;
     case MLSB : m="L"; break;
     case MAM  : m="A"; break;
     case MFM  : m="F"; break;
     default   : m="?"; break;
      
   }
   lcd.print(m);
}
/*
//*--------------------------------------------------------------------------------------------
//* showSplit
//* show Split status
//*--------------------------------------------------------------------------------------------
void showSplit() {

  lcd.setCursor(14,0);
  if (getWord(FT817,SPLIT)==true) {
     lcd.write(byte(8));
  } else {
     lcd.print(" ");   
  }
}
*/
/*
//*--------------------------------------------------------------------------------------------
//* showRIT
//* show RIT value
//*--------------------------------------------------------------------------------------------
void showRIT() {
int r;
  
  (getWord(FT817,VFO)==false ? lcd.setCursor(11,0) : lcd.setCursor(11,1));  
  if (getWord(FT817,RIT)==false) {
     r=0;    
     sprintf(hi,"  ");
  } else {
     r=(ritVFO/100);
     sprintf(hi,"%+02d",r);
  }
  lcd.print(hi);
  
}
*/
/*
//*--------------------------------------------------------------------------------------------
//* showBar
//* show Bar status (NOT IMPLEMENTED YET)
//*--------------------------------------------------------------------------------------------
void showBar() {

//*---- needs to further be developed as a general purpose meter method
int s=SNR & 0x07;
  lcd.setCursor(15,0);
  switch(s) {
      case 0 : lcd.print(" ");break;
      case 1 : lcd.print(" ");break;
      case 2 : lcd.write(byte(0xa1));break;
      case 3 : lcd.write(byte(2));break;
      case 4 : lcd.write(byte(3));break;
      case 5 : lcd.write(byte(4));break;
      case 6 : lcd.write(byte(255));break;
      case 7 : lcd.write(byte(255));break;
   }
    
}
*/
//*--------------------------------------------------------------------------------------------
//* showVOX
//* show VOX status (valid only for LSB/USB)
//*--------------------------------------------------------------------------------------------
void showVOX() {
  lcd.setCursor(14,1);
  if (getWord(FT817,VOX)==true) {
     lcd.print("V");
  } else {
     lcd.print(" ");   
  }
  
}
//*--------------------------------------------------------------------------------------------
//* showPanel
//* show frequency or menu information at the display
//*--------------------------------------------------------------------------------------------
void showPanel() {
  
   if (getWord(MSW,CMD)==false) {
      lcd.clear();
      showPTT();
      showMode();
      //showSplit();
      //showBar();
      //showRIT();
      showVOX();
      return;
   }
}
//*--------------------------------------------------------------------------------------------
//* showUnderline
//* show underline under the frequency which is changing
//*--------------------------------------------------------------------------------------------
void showUnderline() {

int c=7;
  switch(vfostep) {
     case VFO_STEP_1MHz   : c=3; break;
     case VFO_STEP_100KHz : c=5; break;
     case VFO_STEP_10KHz  : c=6; break;
     case VFO_STEP_1KHz   : c=7; break;
     case VFO_STEP_100Hz  : c=9; break;
  }
  lcd.setCursor(c,(byte)getWord(FT817,VFO));lcd.cursor();
}
//*---------------------------------------------------------------------------------------------------
//* update the frequency for a given VFO
//*---------------------------------------------------------------------------------------------------
void updateVFO(long int vstep) {
   
   vfo=vfo+vstep;
   if (vfo>VFO_HIGHER) {
      vfo=VFO_HIGHER;
   } else {
      if (vfo<VFO_LOWER) {
         vfo=VFO_LOWER;
      }
   }
}
//*---------------------------------------------------------------------------------------------------
//* compute the current frequency in string format to display
//*---------------------------------------------------------------------------------------------------
void computeVFO(long int f, FSTR* v) {
  
  (*v).millions =       int(f / 1000000);
  (*v).hundredthousands = ((f / 100000) % 10);
  (*v).tenthousands =     ((f / 10000) % 10);
  (*v).thousands =        ((f / 1000) % 10);
  (*v).hundreds =         ((f / 100) % 10);
  (*v).tens =             ((f / 10) % 10);
  (*v).ones =             ((f / 1) % 10);
  return; 
   
}
//*---------------------------------------------------------------------------------------------------
//* Show the frequency of a given VFO
//*---------------------------------------------------------------------------------------------------
void showQRG() {
  
  FSTR v;  
    
  computeVFO(vfo,&v);
  lcd.setCursor(2,0);
    
  if (v.millions <10) {
    lcd.print(" ");
  }
  
  lcd.print(v.millions);
  lcd.print(".");
  lcd.print(v.hundredthousands);
  lcd.print(v.tenthousands);
  lcd.print(v.thousands);
  lcd.print(",");
  lcd.print(v.hundreds);

  lcd.setCursor(10,0);
  lcd.write(byte(0));

}
//*--------------------------------------------------------------------------------------------
//* showFreq
//* show frequency at the display
//*--------------------------------------------------------------------------------------------
void showFreq() {

//*---- Prepare to display
  lcd.setCursor(0, 0);

//*--- show VFO, frequency and underline


  showQRG();   
  showUnderline();
  
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//* Show Mark of a recent frequency change on a given VFO
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void showMark(char* m){
  
      lcd.setCursor(1,0);
      if (strcmp(m,"<")==0) {
         lcd.print((char)0x7f);
      } else {
         lcd.print((char)0x7e);
      }
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//* Unshow Mark of a recent frequency change on a given VFO
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void unshowMark(){
     
      lcd.setCursor(1,0);
      lcd.print(" ");
      showUnderline(); 
}
