// LiquidCrystal I2C https://github.com/johnrickman/LiquidCrystal_I2C
#include <LiquidCrystal_I2C.h>

// RotaryEncoder https://github.com/mathertel/RotaryEncoder
#include <RotaryEncoder.h>

// Etherkit Si5351 https://github.com/etherkit/Si5351Arduino
#include <si5351.h>

#include <Arduino.h>
#include <Wire.h>

// in hundredths of hertz
#define CAL_FACTOR 150000
#define TONE_FREQ 30000ULL
#define IF_FREQ 400000000ULL
#define BFO_FREQ ( IF_FREQ + TONE_FREQ )
#define VFO_FREQ ( ( center_freq * SI5351_FREQ_MULT ) + IF_FREQ )

// in hertz
uint64_t center_freq = 7000000ULL;
uint64_t freq_step = 500ULL;

int rotary_encoder_pos;

#define ROTARY_PIN1 7
#define ROTARY_PIN2 8
#define ROTARY_SWITCH 9

Si5351 si5351;
LiquidCrystal_I2C lcd(0x27,16,2);
RotaryEncoder encoder(ROTARY_PIN1, ROTARY_PIN2, RotaryEncoder::LatchMode::TWO03);

bool rotary_switch = LOW;
bool freq_changed = false;
bool display_changed = true;

void setup_si5351()
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

  si5351.set_freq(VFO_FREQ, SI5351_CLK0);
  si5351.set_freq(BFO_FREQ, SI5351_CLK1);
  si5351.set_freq(IF_FREQ, SI5351_CLK2);

  si5351.update_status();
}

void setup_lcd()
{
  lcd.init();
  lcd.backlight();

  // Print a message to the LCD.
  lcd.setCursor(1,0);
  lcd.print("Hello, world!");
}

void setup_rotary_encoder()
{
  encoder.tick();
  rotary_encoder_pos = encoder.getPosition();

  pinMode(ROTARY_SWITCH, INPUT);
}

void read_rotary_encoder()
{
  encoder.tick();
  int newPos = encoder.getPosition();
  int delta = 0;

  switch (encoder.getDirection())
  {
    case RotaryEncoder::Direction::CLOCKWISE:
      delta = (newPos - rotary_encoder_pos + 16) % 16;
      break;
    case RotaryEncoder::Direction::COUNTERCLOCKWISE:
      delta = (newPos - rotary_encoder_pos - 16) % 16;
      break;
    case RotaryEncoder::Direction::NOROTATION:
      break;
  }

  if (delta == 0) return;

  center_freq += delta * freq_step;
  freq_changed = true;

  rotary_encoder_pos = newPos;
}

void read_rotary_switch()
{
  bool newState = digitalRead(ROTARY_SWITCH);

  if (rotary_switch == newState) return;

  if (newState == HIGH) {
    freq_step = (freq_step + 10) % 1000;
  }

  rotary_switch = newState;
}

void update_vfo()
{
  if (!freq_changed) return;

  si5351.set_freq(VFO_FREQ, SI5351_CLK0);
  si5351.update_status();
  
  freq_changed = false;
  display_changed = true;
}

void update_display()
{
  if (!display_changed) return;
  
  lcd.setCursor(0,0);
  lcd.print((unsigned long)center_freq);

  display_changed = false;
}

void setup()
{
  setup_si5351();
  setup_lcd();
  setup_rotary_encoder();
  delay(1000);
}

void loop()
{
  read_rotary_encoder();
  read_rotary_switch();
  update_vfo();
  update_display();
  delay(100);
}
