#include "ESP8266mDNS.h"
#include "ESP8266WebServer.h"
#include "ESP8266WiFi.h"
#define FASTLED_ALLOW_INTERRUPTS 0
#include "FastLED.h"

#define BRIGHTNESS  196
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

DEFINE_GRADIENT_PALETTE(whites) {
  0, 255, 255, 255,
  128, 255, 214, 170,
  255, 255, 197, 143
};

// Red, green, blue, yellow, purple.
DEFINE_GRADIENT_PALETTE(xmas) {
  0, 255, 0, 0,
  64, 0, 255, 0,
  128, 0, 0, 255,
  192, 255, 255, 0,
  255, 153, 50, 204
};

bool autoChangePalette = false;
const TProgmemRGBGradientPalettePtr palettes[] = {
  xmas,
  whites,
  (TProgmemRGBGradientPalettePtr)RainbowColors_p,
  green_purple,
  orange_green,
  orange
};

uint8_t current_palette_num = 0;
CRGBPalette16 palette = palettes[current_palette_num];

uint8_t current_pattern_num = 0;
uint8_t cycle_counter = 0;
double speed_override = 2;
bool wifi_connected = false;
ESP8266WebServer server(80);

void setup() {
  pinMode(ONBOARD_PIN , OUTPUT);
  
  delay(500);
  
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  fill_solid(leds, NUM_LEDS, CRGB::Black);

  Serial.begin(115200);
  WiFi.hostname("tree");
  WiFi.begin("cocacola", "football");
}

bool autoChangePattern = true;
typedef void (*SimplePatternList[])();
SimplePatternList patterns = { confetti, ribbons, crawl, juggle };
  
void loop() {
  patterns[current_pattern_num]();
  cycle_counter++;

  EVERY_N_SECONDS(20) {
    if (autoChangePattern) {
      nextPattern();
    }
  }
  EVERY_N_SECONDS(80) {
    if (autoChangePalette) {
      nextPalette();
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (!wifi_connected) {
      // Moving from not connected to connected.
      
      // Set up DNS.
      if (!MDNS.begin("tree")) { Serial.println("DNS failed."); }
      else { Serial.println("DNS set."); }
      
      // Set up server.
      server.on("/", handleIndex);
      server.on("/set", HTTP_GET, handleSet);
      server.onNotFound(handleNotFound);
      server.begin();
    }
    wifi_connected = true;
    server.handleClient();
  }

  EVERY_N_SECONDS(5) {
    Serial.print("wifi status: ");
    if (WiFi.status() == WL_CONNECTED) { Serial.print(WiFi.localIP()); }
    else { Serial.print(WiFi.status()); }
    Serial.println();
  }

  // Onboard LED indicator.
  EVERY_N_MILLISECONDS(500) { digitalWrite(ONBOARD_PIN, HIGH); /* On */}
  if (WiFi.status() == WL_CONNECTED) {
    // Off
    EVERY_N_SECONDS(10) { digitalWrite(ONBOARD_PIN, LOW); /* Off */}
  } else {
    // Off
    EVERY_N_SECONDS(1) { digitalWrite(ONBOARD_PIN, LOW); /* Off */}
  }

  FastLED.show();  
  FastLED.delay(1000/FRAMES_PER_SECOND);
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

/////////////////////////////////////////////////////////
// LED

void nextPattern() {
  current_pattern_num = addmod8(current_pattern_num, 1, ARRAY_SIZE(patterns));
}

void nextPalette() {
  current_palette_num = addmod8(current_palette_num , 1, ARRAY_SIZE(palettes));
  palette = palettes[current_palette_num];
}

void setDefaultPalette(uint8_t paletteIndex) {
  current_palette_num = min(paletteIndex, (uint8_t)(ARRAY_SIZE(palettes) - 1));
  palette = palettes[current_palette_num];
}

int8_t crawl_index = 0;

void crawl() {
  fadeToBlackBy(leds, NUM_LEDS, 8);
  accum88 bpm = 5 * speed_override;
  uint16_t led = beatsin16(bpm, 0, NUM_LEDS - 1);
  // Only set this index once.
  if (crawl_index != led) {
    crawl_index  = led;
    leds[led] += ColorFromPalette(palette, random8(), BRIGHTNESS, NOBLEND);
  }
}

void confetti()  {
  #define CONFETTI_MS 150
  fadeToBlackBy(leds, NUM_LEDS, 1);
  EVERY_N_MILLIS_I(timer, CONFETTI_MS) {
    int pos = random16(NUM_LEDS);
    leds[pos] += ColorFromPalette(palette, random8(), BRIGHTNESS, NOBLEND);
  }
  timer.setPeriod(CONFETTI_MS / speed_override);
}

int8_t juggle_index = 0;

void juggle() {
  // Dots weaving in and out of sync with each other.
  fadeToBlackBy(leds, NUM_LEDS, 5);
  accum88 bpm = 1 * speed_override;
  //uint16_t location = beatsin16(bpm, 0, NUM_LEDS - 1);
  uint8_t location = beat8(bpm) % NUM_LEDS;
  if (juggle_index != location) {
    juggle_index = location;
    uint8_t location_1 = location;
    uint8_t location_2 = addmod8(location, 5, NUM_LEDS);
    uint8_t location_3 = addmod8(location_1, NUM_LEDS / 2, NUM_LEDS);
    uint8_t location_4 = addmod8(location_2, NUM_LEDS / 2, NUM_LEDS);
    CRGB color1 = ColorFromPalette(palette, random8(), BRIGHTNESS, NOBLEND);
    CRGB color2 = ColorFromPalette(palette, random8(), BRIGHTNESS, NOBLEND);
    leds[location_1] |= color1;
    leds[location_3] |= color1;
    leds[location_2] |= color2;
    leds[location_4] |= color2;
  }
}

int8_t ribbons_index = 0;

// Fill the dots one after the other with a color.
void ribbons() {
  fadeToBlackBy(leds, NUM_LEDS, 16);
  accum88 bpm = 5 * speed_override;
  uint16_t led = beatsin16(bpm, 0, NUM_LEDS - 1);
  // Only set this index once.
  if (ribbons_index != led) {
    ribbons_index  = led;
    leds[led] += ColorFromPalette(palette, random8(), BRIGHTNESS, NOBLEND);
  }
}

int8_t rainbow_crawl_index = 0;

void rainbow_crawl() {
  EVERY_N_MILLISECONDS(20) {
    uint8_t hue = mod8(rainbow_crawl_index, 255);
    fill_rainbow(leds, NUM_LEDS, hue);
    rainbow_crawl_index += 1.2 * speed_override;
  }
}

void just_orange() {
  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette((CRGBPalette16)orange, random8(), BRIGHTNESS, LINEARBLEND);
  }
}

/////////////////////////////////////////////////////////
// HTTP

// http://tree.local/set?speed=5

void handleIndex() {
  server.send(200, "text/plain", "true");
}

void handleSet() {
  int argCount = server.args();
  Serial.print("arg count: ");
  Serial.println(argCount);

  if (argCount == 0) {
    server.send(400, "text/plain", "Invalid request, requires arguments.");
    return;
  }

  bool success = true;

  for (int i = 0; i < argCount; i++) {
    String argName = server.argName(i);
    Serial.print("Found argument: ");
    Serial.println(argName);
    if (argName.equals("speed")) {
      success = success && updateSpeed(server.arg(i));
    }
    if (argName.equals("autoChangePalette")) {
      success = success && updateChangePalette(server.arg(i));
    }
    if (argName.equals("palette")) {
      success = success && updatePalette(server.arg(i));
    }
  }

  if (success) {
    server.send(200, "text/plain", "success");
  } else {
    server.send(503, "text/plain", "Failed to parse args");
  }
}

bool updateSpeed(String argval) {
  if (argval == NULL) {
    server.send(400, "text/plain", "Invalid request, speed requires nonnull argument.");
    return false;
  }

  float speed = argval.toFloat();
  if (speed == 0) {
    server.send(400, "text/plain", "Could not parse speed as float (0 is invalid).");
    return false;
  }

  Serial.print("Setting speed: ");
  Serial.println(speed);
  speed_override = speed;
  
  return true;
}

bool updateChangePalette(String argval) {
  if (argval == NULL) {
    server.send(400, "text/plain", "Invalid request, autoChangePalette requires nonnull argument.");
    return false;
  }

  bool shouldChange = (bool)argval.toInt();

  Serial.print("Setting shouldChange: ");
  Serial.println(shouldChange ? "true" : "false");
  autoChangePalette = shouldChange;
  if (autoChangePalette ) {
    nextPalette();
  }
  
  return true;
}

bool updatePalette(String argval) {
  if (argval == NULL) {
    server.send(400, "text/plain", "Invalid request, paletteNum requires integer argument.");
    return false;
  }

  uint8_t num = argval.toInt();

  Serial.print("Setting palette num: ");
  Serial.println(num);
  setDefaultPalette(num);
  
  return true;
}

void handleNotFound() {
  server.send(404, "text/plain", "404");
}
