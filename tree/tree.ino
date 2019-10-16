#include "FastLED.h"

FASTLED_USING_NAMESPACE

#define BRIGHTNESS  255
#define DATA_PIN    D2
#define LED_TYPE    WS2811
#define COLOR_ORDER RGB
#define NUM_LEDS    100
#define FRAMES_PER_SECOND  120
#define ONBOARD_PIN 2

CRGB leds[NUM_LEDS];

DEFINE_GRADIENT_PALETTE(orange_green) {
  0, 255, 84, 0, // Orange
  255, 0, 196, 32, // Green
};

DEFINE_GRADIENT_PALETTE(green_purple) {
  0, 255, 0, 128, // Purple
  255, 0, 196, 32, // Green
};

const TProgmemRGBGradientPalettePtr palettes[] = { green_purple, orange_green };
uint8_t current_palette_num = 0;
CRGBPalette16 palette = palettes[current_palette_num];

uint8_t current_pattern_num = 0;
uint8_t cycle_counter = 0;

void setup() {
  pinMode(ONBOARD_PIN , OUTPUT);
  
  delay(500);
  
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  fill_solid(leds, NUM_LEDS, CRGB::Black);
}

typedef void (*SimplePatternList[])();
SimplePatternList patterns = { confetti, slow };
  
void loop() {
  patterns[current_pattern_num]();
  cycle_counter++;

  FastLED.show();  
  FastLED.delay(1000/FRAMES_PER_SECOND);

  EVERY_N_SECONDS(20) {
    nextPattern();
  }
  EVERY_N_SECONDS(40) {
    nextPalette();
  }

  // Onboard LED indicator.
  EVERY_N_SECONDS(10) {
    digitalWrite(ONBOARD_PIN, LOW);
  }
  EVERY_N_MILLISECONDS(500) {
    digitalWrite(ONBOARD_PIN, HIGH);
  }
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern() {
  current_pattern_num = addmod8(current_pattern_num, 1, ARRAY_SIZE(patterns));
}

void nextPalette() {
  current_palette_num = addmod8(current_palette_num , 1, ARRAY_SIZE(palettes));
  palette = palettes[current_palette_num];
}

int8_t slow_index = 0;
void slow() {
  fadeToBlackBy(leds, NUM_LEDS, 10);
  accum88 bpm = 4;
  uint16_t led = beatsin16(bpm, 0, NUM_LEDS - 1);
  // Only set this index once.
  if (slow_index != led) {
    slow_index = led;
    leds[led] += ColorFromPalette(palette, random8(), 255, NOBLEND);
  }
}

void confetti()  {
  fadeToBlackBy(leds, NUM_LEDS, 4);
  if (cycle_counter % 16 == 0) {
    int pos = random16(NUM_LEDS);
    leds[pos] += ColorFromPalette(palette, random8(), 255, NOBLEND);
  }
}

void juggle() {
  // Dots weaving in and out of sync with each other
  fadeToBlackBy(leds, NUM_LEDS, 20);
  for( int i = 0; i < 3; i++) {
    leds[beatsin16(i+7, 0, NUM_LEDS - 1)] |= ColorFromPalette(palette, random8(), 255, NOBLEND);
  }
}

