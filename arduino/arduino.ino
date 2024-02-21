#include "si5351.h"
#include "Wire.h"

// in hundredths of hertz
#define CAL_FACTOR 150000
#define IF_FREQ 400000000ULL
#define TONE_FREQ 30000ULL
#define BFO_FREQ (IF_FREQ + TONE_FREQ)

uint64_t vfo_freq = 1000000000ULL;

Si5351 si5351;

void setup()
{
  // Start serial and initialize the Si5351
  Serial.begin(9600);

  // The crystal load value needs to match in order to have an accurate calibration
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  si5351.output_enable(SI5351_CLK0, 1);
  si5351.output_enable(SI5351_CLK1, 1);
  si5351.output_enable(SI5351_CLK2, 1);

  // Start on target frequency
  si5351.set_correction(CAL_FACTOR, SI5351_PLL_INPUT_XO);
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);

  si5351.set_freq(vfo_freq, SI5351_CLK0);
  si5351.set_freq(BFO_FREQ, SI5351_CLK1);
  si5351.set_freq(IF_FREQ, SI5351_CLK2);

  si5351.update_status();
  delay(500);
}

void loop()
{
  si5351.update_status();
  delay(500);
}
