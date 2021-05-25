//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//*                     CAT SUPPORT
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
#ifdef CAT
//#*---------------------------------------------------------------------------
//#* bcdToDec
//#* Convert nibble
//#*---------------------------------------------------------------------------
long int bcdToDec(int v) {
  return  (v/16*10) + (v%16);
} 
//#*--------------------------------------------------------------------------
//#* decToBcd
//#* Convert nibble
//#*-------------------------------------------------------------------------
int decToBcd(int v){
  return  (v/10*16) + (v%10);
}
//#*---------------------------------------------------------------------------
//#* hex2str
//#* Convert byte to Hex string
//#*---------------------------------------------------------------------------
void hex2str(char* r, byte* b,int l) {

      for(int j = 0; j < l; j++) {
         sprintf(&r[2*j], "%02X",b[j]);
      }
}
//#*---------------------------------------------------------------------------
//#* BCD2Dec
//#* Convert 4 BCD bytes to integer
//#*---------------------------------------------------------------------------
long int BCD2Dec(byte* BCDBuf) {

    long int f=0;
    f=f+bcdToDec(BCDBuf[0])*1000000;
    f=f+bcdToDec(BCDBuf[1])*10000;
    f=f+bcdToDec(BCDBuf[2])*100;
    f=f+bcdToDec(BCDBuf[3])*1;
    f=f*10;
    return f;
}
//#*---------------------------------------------------------------------------
//#* dec2BCD
//#* Convert convert frequency integer into BCD
//#*---------------------------------------------------------------------------
void dec2BCD(byte* BCDBuf, long int f){

    long int fz=f/10;
    long int f0=fz/1000000;
  
    long int x1=fz-f0*1000000;
    long int f1=x1/10000;
  
    long int x2=x1-f1*10000;
    long int f2=x2/100;

    long int x3=x2-f2*100;
    long int f3=x3;

    f0=decToBcd((int)f0);
    f1=decToBcd((int)f1);
    f2=decToBcd((int)f2);
    f3=decToBcd((int)f3);

    BCDBuf[0]=(byte)f0;
    BCDBuf[1]=(byte)f1;
    BCDBuf[2]=(byte)f2;
    BCDBuf[3]=(byte)f3;


 }

//#*---------------------------------------------------------------------------
//#* sendSerial
//#* send buffer over serial link
//#*---------------------------------------------------------------------------
void sendSerial(byte* BCDBuf,int len) {
int bufLen=0;
    for(int j = 0; j < len; j++) {  
       Serial.write((char)BCDBuf[j]);
    }
}
//#*---------------------------------------------------------------------------
//#* processCAT
//#* After a full command has been received process it
//#*---------------------------------------------------------------------------
void processCAT(byte* rxBuffer) {

    byte BCDBuf[6];
    char buffer[18];

    BCDBuf[0]=0x00;
    BCDBuf[1]=0x00;
    BCDBuf[2]=0x00;
    BCDBuf[3]=0x00;
    BCDBuf[4]=0x00;
    BCDBuf[5]=0x00;

    switch(rxBuffer[4]) {
      case 0x01:  {        //* Set Frequency
       long int f=BCD2Dec(&rxBuffer[0]);
       SendFrequency(f);
       showFreq();
       
       BCDBuf[4]=0x01;
       BCDBuf[0]=0x00;
       sendSerial(&BCDBuf[0],1);
       return;}
      
      case 0x03:  {        //* Get Frequency and Mode
       //long int f=(getWord(FT817,VFO)==false ? vfo[VFOA] : vfo[VFOB]);  //**CAMBIAR AQUI**
       long int f= vfo;  //**CAMBIAR AQUI**
       
       dec2BCD(&BCDBuf[0],f);
       BCDBuf[4]=MODE;
       sendSerial(&BCDBuf[0],5);
       return;}
       
      case 0x00: {   //* LOCK status flip  Not really implemented
       //if(getWord(FT817,LOCK)==true) {
       //  BCDBuf[0]=0xF0;
       //} else {
       //  BCDBuf[0]=0x00;
       //}
       BCDBuf[0]=0x00;
       //setWord(&FT817,LOCK,true);
       sendSerial(&BCDBuf[0],1);
       //sendStatus();
       return; }
      case 0x02: { //* SPLIT status flip (ver si no se puede anular)
       if(getWord(FT817,SPLIT)==true) {
         setWord(&FT817,SPLIT,false);
         BCDBuf[0]=0xF0;
       } else {
         setWord(&FT817,SPLIT,true);
         BCDBuf[0]=0x00;
       }      
       sendSerial(&BCDBuf[0],1);
       showSplit();
       return;}
      case 0x05: { //* RIT status flip (ver si no se puede eliminar
       if(getWord(FT817,RIT)==true) {
         setWord(&FT817,RIT,false);
         BCDBuf[0]=0xF0;
       } else {
         setWord(&FT817,RIT,true);
         BCDBuf[0]=0x00;
       }
       
       sendSerial(&BCDBuf[0],1);
       showRIT();
       return;}
       
      case 0x07: {      //* Transceiver mode change
       byte mode=rxBuffer[0] & 0xff;       //* Prevent invalid values to be set
       if (mode != MLSB && mode != MUSB && mode != MCW && mode != MCWR && mode != MAM && mode != MWFM && mode != MFM && mode != MDIG && mode != MPKT) {
          hex2str(&buffer[0],&rxBuffer[0],1);         
          return;
       }
       MODE=mode;   //* Prevenir que cambie a alguno no soportado
       BCDBuf[0]=0x00;      
       sendSerial(&BCDBuf[0],1);
       showMode();
       return;}

      case 0x08: {     //* Undocummented feature returns 0x00 if transceiver unkeyed 0xF0 if keyed
       if(getWord(FT817,PTT)==true) {
         BCDBuf[0]=0xF0;
       } else {
         BCDBuf[0]=0x00;
       }
       setWord(&FT817,PTT,true);
       sendSerial(&BCDBuf[0],1);
       return;}

      case 0x80: {      //* Turn the LOCK off
       if(getWord(FT817,LOCK)==false) {
         BCDBuf[0]=0xF0;
       } else {
         BCDBuf[0]=0x00;
       }
       setWord(&FT817,LOCK,false);
       sendSerial(&BCDBuf[0],1);
       return; }

      case 0x81:  {
       BCDBuf[0]=0x00;
       (getWord(FT817,VFO)==false?setWord(&FT817,VFO,true):setWord(&FT817,VFO,false));
       BCDBuf[0]=0x00;
       sendSerial(&BCDBuf[0],1);
       return;}
      case 0x82: {      //* Turn the SPLIT off
       if(getWord(FT817,SPLIT)==false) {
         BCDBuf[0]=0xF0;
       } else {
         BCDBuf[0]=0x00;
       }
       setWord(&FT817,SPLIT,false);
       sendSerial(&BCDBuf[0],1);
       showSplit();
       return; }
      case 0x85: {      //* Turn the RIT off
       if(getWord(FT817,RIT)==false) {
         BCDBuf[0]=0xF0;
       } else {
         BCDBuf[0]=0x00;
       }
       setWord(&FT817,RIT,false);
       sendSerial(&BCDBuf[0],1);
       showSplit();
       return; }
      case 0x88: {      //* Turn the TX On
       if(getWord(FT817,PTT)==false) {
         BCDBuf[0]=0xF0;
       } else {
         BCDBuf[0]=0x00;
       }
       setWord(&FT817,PTT,false);
       sendSerial(&BCDBuf[0],1);
       showPTT();
       return; }
      case 0xBB:  {     //* only address 0x55 is supported the rest is answered nominally without modifications
       if (rxBuffer[1]==0x55) {
          BCDBuf[0]=0x80;
          BCDBuf[1]=0x00;
          if (getWord(FT817,VFO)==true) {
             BCDBuf[0]=BCDBuf[0] | 0x01;
          }
          sendSerial(&BCDBuf[0],2);
          return;
       } 
       if (rxBuffer[1]==0x64) {
          BCDBuf[0]=0x00;
          BCDBuf[1]=0x00;
          sendSerial(&BCDBuf[0],2);
          return;
       } 
       return;
       }
      
      case 0xF5: {      //* Set Clarifier Frequency (not implemented yet)
       byte offsetBuf[5];
       offsetBuf[2]=rxBuffer[2];
       offsetBuf[3]=rxBuffer[3];
       offsetBuf[0]=0x00;
       offsetBuf[1]=0x00;
       offsetBuf[4]=0xF5;
       int ofs=BCD2Dec(&offsetBuf[0]);
       ofs=ofs/10;
       /*
       if (rxBuffer[0]==0x00) {
          RITOFS=+ofs;
       } else {
          RITOFS=(-1)*ofs;
       }
       */
       BCDBuf[0]=0x00;
       sendSerial(&BCDBuf[0],1);
       //sendStatus();
       return; }  

      case 0xE7:  {     //* Receiver Status
       //int RX;
       /*
       if (METER>0x0F) {
          float prng= (float)std::rand();
          float pmax= (float)RAND_MAX;
          RX  = (SMETERMAX*(float(prng/pmax)));
          if (RX<0x06) { RX=0x06; }  // this is actually a simulation to keep the indicator alive, no signals below S6 are realistic on HF....
       } else {
          RX  = METER;
       }
       */

//*----- S-Meter signal coded for command 0xE7
//*----- xxxx0000 S0
//*----- xxxx0001 S1
//*----- xxxx0010 S2
//*----- xxxx0011 S3
//*----- xxxx0100 S4
//*----- xxxx0101 S5
//*----- xxxx0110 S6
//*----- xxxx0111 S7
//*----- xxxx1000 S8
//*----- xxxx1001 S9
//*----- xxxx1010 S9+10dB
//*----- xxxx1011 S9+20dB
//*----- xxxx1100 S9+30dB
//*----- xxxx1101 S9+40dB
//*----- xxxx1110 S9+50dB
//*----- xxxx1111 S9+60dB

       BCDBuf[0]=((int)RX & 0x0f) | 0x00;     //* Either fake level or extracted from receiver
       sendSerial(&BCDBuf[0],1);
       return;}

      case 0xF7:  {     //* Transmitter status
       BCDBuf[0]=0x00;
       byte POWER=0;
       if (getWord(FT817,PTT)==true) {

          (getWord(FT817,PTT)==false  ? BCDBuf[0]=BCDBuf[0] | 0B10000000 : BCDBuf[0]=BCDBuf[0] & 0B01111111);
          (getWord(FT817,SPLIT)==true ? BCDBuf[0]=BCDBuf[0] | 0B00100000 : BCDBuf[0]=BCDBuf[0] & 0B11011111);
          BCDBuf[0]=BCDBuf[0] & 0B10111111;

          //TX=POWER*2;
          BCDBuf[0]=BCDBuf[0] | ((((int)TX) & 0x0f));

       } else {
          BCDBuf[0]=0B10000000;
       }
       sendSerial(&BCDBuf[0],1);
       return;}

      case 0xBD:  {   //* TX Metering

       if (getWord(FT817,PTT)==false) {
          BCDBuf[0]=0x00;
          sendSerial(&BCDBuf[0],1);
          return;
       }
       
       
       //BCDBuf[0]=BCDBuf[0] | ((((POWER*2) & 0x0f) << 1) & 0xf0) | 0x01;
       BCDBuf[1]=0x11;
       sendSerial(&BCDBuf[0],2);
       return; }

      default:    {
                  }
     } 

}
//*-------------------------------------------------------------------------
//* get
//* get a character from the  serial port and process it
//*-------------------------------------------------------------------------
void get() {

    char  buf[16];
    int maxFrame=4;
    while(Serial.available()) {
       byte k=Serial.read();

       rxBuffer[n]=k;
             
       n++;
       if (n==5) {
          char buffer[18];
          //hex2str(&buffer[0],&rxBuffer[0],n);
          //(TRACE>=0x03 ? fprintf (stderr,"%s::get() Received Serial hex2str (%s)\n",PROGRAMID,(char*) &buffer[0]) : _NOP);
          if (rxBuffer[5]==0x03) {

          }    
          
          processCAT(&rxBuffer[0]);
          //fflush (stdout) ;
          n=0;
          maxFrame--;
          if (maxFrame==0) { 
             return;  //If processed one command don't get stuck here
          }
       }
    }

}
#endif CAT
