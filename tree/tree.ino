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
DEFINE_GRADIENT_PALETTE(orange) {
  0, 32, 8, 0, // Very Dark Orange
  255, 255, 84, 0 // Orange
};

const TProgmemRGBGradientPalettePtr palettes[] = { green_purple, orange_green, orange };
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
SimplePatternList patterns = { confetti, crawl, juggle, just_orange };
  
void loop() {
  patterns[current_pattern_num]();
  cycle_counter++;

  FastLED.show();  
  FastLED.delay(1000/FRAMES_PER_SECOND);

  EVERY_N_SECONDS(20) {
    nextPattern();
  }
  EVERY_N_SECONDS(80) {
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

int8_t crawl_index = 0;
void crawl() {
  fadeToBlackBy(leds, NUM_LEDS, 10);
  accum88 bpm = 5;
  uint16_t led = beatsin16(bpm, 0, NUM_LEDS - 1);
  // Only set this index once.
  if (crawl_index != led) {
    crawl_index  = led;
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

int8_t juggle_index = 0;
void juggle() {
  // Dots weaving in and out of sync with each other
  fadeToBlackBy(leds, NUM_LEDS, 20);
  accum88 bpm = 2;
  uint16_t loc = beatsin16(bpm, 0, NUM_LEDS - 1);
  if (juggle_index != loc) {
    juggle_index = loc;
    leds[addmod8(juggle_index, 2, NUM_LEDS)] |= ColorFromPalette(palette, 255, 255, NOBLEND);
    leds[juggle_index] |= ColorFromPalette(palette, 0, 255, NOBLEND);
  }
}

void just_orange() {
  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette((CRGBPalette16)orange, random8(), 255, LINEARBLEND);
  }
}

