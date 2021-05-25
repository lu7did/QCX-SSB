#include <asserts.h>
#include <errors.h>
const unsigned long long pll_freq = 87000000000ULL;
//=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
//*                                              Si5351 Controller
//=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=

void initDDS() {
  
    // Initialize the DDS
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
/*
  si5351.set_correction(31830, SI5351_PLL_INPUT_XO);      // Set to specific Si5351 calibration number
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  
  //si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
  //si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_8MA);
  si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_8MA);
  
  si5351.set_freq((freq * 100ULL), SI5351_CLK0);
  
  //si5351.set_freq((freq * 100ULL), SI5351_CLK1);
  //si5351.set_freq((freq * 100ULL), SI5351_CLK2);
  
  //si5351.set_freq((BFO_freq * 100ULL), SI5351_CLK2);
*/
//si5351.set_correction(-67);



  uint8_t temp = si5351.si5351_read(16);
  Serial.println(temp, HEX);
  temp = si5351.si5351_read(17);
  Serial.println(temp, HEX);

  si5351.set_pll(pll_freq, SI5351_PLLA);
  //si5351.set_pll(pll_freq, SI5351_PLLB);
  si5351.si5351_write(16, 0x80);
  si5351.si5351_write(17, 0x80);
  //si5351.si5351_write(18, 0x80);
  si5351.set_ms_source(SI5351_CLK0, SI5351_PLLA);
  si5351.set_ms_source(SI5351_CLK1, SI5351_PLLA);
  si5351.set_ms_source(SI5351_CLK2, SI5351_PLLA);
  
  si5351.set_freq_manual((freq * 100ULL), pll_freq, SI5351_CLK0);
  si5351.set_freq_manual((freq * 100ULL), pll_freq, SI5351_CLK1);
  si5351.set_freq_manual((freq * 100ULL), pll_freq, SI5351_CLK2);

  si5351.set_phase(SI5351_CLK0, 0);
  si5351.set_phase(SI5351_CLK1, 0);
  si5351.set_phase(SI5351_CLK2, 0);
    
  si5351.si5351_write(16, 0x0c);
  si5351.si5351_write(17, 0x0c);
  si5351.si5351_write(18, 0x0c);
  si5351.pll_reset(SI5351_PLLA); 
  
  Serial.println("Si5351 initialized\n");
}

void SendFrequency(int32_t f) {

   //si5351.set_freq((f * 100ULL), SI5351_CLK0);
  //si5351.set_freq((f * 100ULL)), pll_freq, SI5351_CLK0);
  si5351.set_freq_manual((f * 100ULL), pll_freq, SI5351_CLK2);

   

}

