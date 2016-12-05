#include<FastLED.h>
#include "GradientPalettes.h"

#define NUM_MODES 22
#define LIGHT_TEST 0

#define MAX_BRIGHT 100

byte state = 1;
int index = 0;               //-LED INDEX (0 to NUM_LEDS-1)

byte thisbright = 128;
byte thissat = 255;
int thisdelay = 20;          
byte thisstep = 10;           
byte thishue = 0;

byte demoMode = 0;
#define DEMO_COUNT 1600;
int demoStateCountdown;
byte interrupt = 0;

// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100
uint8_t cooling = 49;

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
uint8_t sparking = 60;

uint8_t speed = 30;

///////////////////////////////////////////////////////////////////////

// Forward declarations of an array of cpt-city gradient palettes, and
// a count of how many there are.  The actual color palette definitions
// are at the bottom of this file.
extern const TProgmemRGBGradientPalettePtr gGradientPalettes[];

uint8_t gCurrentPaletteNumber = 0;

CRGBPalette16 gCurrentPalette(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White);
CRGBPalette16 gTargetPalette( gGradientPalettes[0] );

CRGBPalette16 IceColors_p = CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White);
CRGBPalette16 RedWhite_p = CRGBPalette16(CRGB::Red, CRGB::White);
uint8_t currentPatternIndex = 0; // Index number of which pattern is current
uint8_t autoplay = 0;

uint8_t autoplayDuration = 10;
unsigned long autoPlayTimeout = 0;

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

CRGB solidColor = CRGB::Blue;

// How many leds are in the strip?
// Be careful with the num leds. The animations that make use of the levels
// will walk right off the led array. The numbers are specific to my led 
// tree. If you are using less leds or are not interested in the layer animations
// you should remove them from the loop or set USE_LEVEL_ANIMATIONS to 0
#define NUM_LEDS 150 // LED Strip
#define LED_TYPE    WS2812B // LED Strip
#define COLOR_ORDER GRB // LED Strip
//#define NUM_LEDS 150 // Tree
//#define LED_TYPE    WS2811 // Tree
//#define COLOR_ORDER RGB // Tree

byte USE_LEVEL_ANIMATIONS = 1; // if 1, will auto advance past level animations
//#define NUM_LEVELS 12
//const PROGMEM uint16_t levels[NUM_LEVELS] = {58, 108, 149, 187, 224, 264, 292, 309, 321, 327, 336, 348};
#define NUM_LEVELS 6
const PROGMEM uint16_t levels[NUM_LEVELS] = {25, 50, 75, 100, 125, 150};

const uint8_t zCoords[NUM_LEDS] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
    52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
    82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
    112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112,
    138, 138, 138, 138, 138, 138, 138, 138, 138, 138, 138, 138, 138, 138, 138, 138, 138,
    150, 150, 150, 150, 150, 150, 150, 150
};

// Data pin that led data will be written out over
#define DATA_PIN 4
// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];

void setup() 
{
  // sanity check delay - allows reprogramming if accidently blowing power w/leds
  delay(2000);

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(MAX_BRIGHT);
  one_color_all(0,0,0);
  FastLED.show();
  delay(1000);
  setDemoMode();
}

void one_color_all(int cred, int cgrn, int cblu) 
{       //-SET ALL LEDS TO ONE COLOR
    for(int i = 0 ; i < NUM_LEDS; i++ ) {
      leds[i].setRGB( cred, cgrn, cblu).nscale8_video(thisbright);
    }
}

void colorTest()
{
  one_color_all(0,0,0);
  FastLED.show();
  delay(1000);
  one_color_all(255,0,0);
  FastLED.show();
  delay(1000);
  one_color_all(0,0,0);
  FastLED.show();
  delay(1000);
  one_color_all(0,255,0);
  FastLED.show();
  delay(1000);
  one_color_all(0,0,0);
  FastLED.show();
  delay(1000);
  one_color_all(0,0,255);
  FastLED.show();
  delay(1000);
}  

void dotTest()
{
   // Move a single white led 
   for(int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1) {
      // Turn our current led on to white, then show the leds
      leds[whiteLed] = CRGB::White;
      // Show the leds (only one of which is set to white, from above)
      FastLED.show();
      // Wait a little bit
      delay(10);
      // Turn our current led back to black for the next loop around
      leds[whiteLed] = CRGB::Black;
   }
}

void lightTest()
{
  colorTest();
  dotTest();
}

void random_burst() 
{
  int rndidx = random16(0, NUM_LEDS);
  int rndhue = random8(0, 255);  
  int rndbright = random8(10, thisbright);
  leds[rndidx] = CHSV(rndhue, thissat, rndbright);
  delay(random8(0, thisdelay));
}

void rgb_propeller() 
{
  thishue = 0;
  index++;
  int ghue = (thishue + 80) % 255;
  int bhue = (thishue + 160) % 255;
  int N3  = int(NUM_LEDS/3);
  int N6  = int(NUM_LEDS/6);  
  int N12 = int(NUM_LEDS/12);  
  for(int i = 0; i < N3; i++ ) {
    int j0 = (index + i + NUM_LEDS - N12) % NUM_LEDS;
    int j1 = (j0+N3) % NUM_LEDS;
    int j2 = (j1+N3) % NUM_LEDS;    
    leds[j0] = CHSV(thishue, thissat, thisbright);
    leds[j1] = CHSV(ghue, thissat, thisbright);
    leds[j2] = CHSV(bhue, thissat, thisbright);    
  }
}

void candycane() 
{
  index++;
  int N3  = int(NUM_LEDS/3);
  int N6  = int(NUM_LEDS/6);  
  int N12 = int(NUM_LEDS/12);  
  for(int i = 0; i < N6; i++ ) {
    int j0 = (index + i + NUM_LEDS - N12) % NUM_LEDS;
    int j1 = (j0+N6) % NUM_LEDS;
    int j2 = (j1+N6) % NUM_LEDS;
    int j3 = (j2+N6) % NUM_LEDS;
    int j4 = (j3+N6) % NUM_LEDS;
    int j5 = (j4+N6) % NUM_LEDS;
    leds[j0] = CRGB(255, 255, 255).nscale8_video(thisbright*.75);
    leds[j1] = CRGB(255, 0, 0).nscale8_video(thisbright);
    leds[j2] = CRGB(255, 255, 255).nscale8_video(thisbright*.75);
    leds[j3] = CRGB(255, 0, 0).nscale8_video(thisbright);
    leds[j4] = CRGB(255, 255, 255).nscale8_video(thisbright*.75);
    leds[j5] = CRGB(255, 0, 0).nscale8_video(thisbright);
  }
}

void rainbow_loop() 
{
  index++;
  thishue = thishue + thisstep;
  if (index >= NUM_LEDS) {index = 0;}
  if (thishue > 255) {thishue = 0;}
  leds[index] = CHSV(thishue, thissat, thisbright);
}

void fill_rainbow_ctrl( struct CRGB * pFirstLED, int numToFill,
                  uint8_t initialhue,
                  uint8_t deltahue,
      uint8_t saturation,
      uint8_t brightness)
{
    CHSV hsv;
    hsv.hue = initialhue;
    hsv.sat = saturation;
    hsv.val = brightness;
    for( int i = 0; i < numToFill; i++) {
        hsv2rgb_rainbow( hsv, pFirstLED[i]);
        hsv.hue += deltahue;
    }
}

void rainbow()
{
  fill_rainbow_ctrl( leds, NUM_LEDS, thishue, thisstep, thissat, thisbright);
  thishue++;
}

void clear_all()
{
  one_color_all(0, 0, 0);
  LEDS.show();
  delay(50);
}

void clear_level(int level)
{
  int startPxl;
  if (level == 0)
    startPxl = 0;
  else
    startPxl = pgm_read_word(&(levels[level-1]));

  for(int i = startPxl; i < pgm_read_word(&(levels[level])); i++)
  {
    leds[i] = CRGB::Black;
  }
}

void light_level_random(int level, byte clearall = 1)
{
  if (clearall)
    clear_all();
  
  int startPxl;
  if (level == 0)
    startPxl = 0;
  else
    startPxl = pgm_read_word(&(levels[level-1]));
    
  for(int i = startPxl; i < pgm_read_word(&(levels[level])); i++)
  {
    leds[i] = CHSV(random8(), random8(50, thissat), random8(50, thisbright));
  }
}

void quick_random_fill()
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CHSV(random8(), random8(), random8(10, thisbright));
  }
}

void light_level(int level, byte red, byte green, byte blue, byte clearall = 1)
{
  if (clearall)
    clear_all();
  
  int startPxl;
  if (level == 0)
    startPxl = 0;
  else
    startPxl = pgm_read_word(&(levels[level-1]));

  for(int i = startPxl; i < pgm_read_word(&(levels[level])); i++)
  {
    leds[i].setRGB(red, green, blue).nscale8_video(thisbright);
  }
}

void drain()
{
  for (int pancakeLevel = 0; pancakeLevel < NUM_LEVELS; pancakeLevel++)
  {
    for (int level = pancakeLevel; level >= 0; level--)
    {
      clear_level(level);
      if (level >= 1) light_level_random(level-1, 0);
            
      LEDS.show();
      delay(75);
    }
  }
}

void pancake()
{
  for (int pancakeLevel = 0; pancakeLevel < NUM_LEVELS; pancakeLevel++)
  {
    if (interrupt) return;
    for (int level = NUM_LEVELS-1; level >= pancakeLevel; level--)
    {
      if (interrupt) return;
      if (level < NUM_LEVELS-1) clear_level(level+1);
      light_level_random(level, 0);
            
      LEDS.show();
      delay(75);
    }
  }
}

void alternate_levels()
{
  static int level = 0;
  light_level(level, dim8_video(random8()), dim8_video(random8()), dim8_video(random8()));
  level++;
  if (level == NUM_LEVELS) level = 0;
}

void random_levels()
{
  int level = random(NUM_LEVELS);
  if (NUM_LEVELS == level) level = 0;
  light_level_random(level, 1);
}

void rainbow_fade() 
{
    thishue++;
    if (thishue > 255) {thishue = 0;}
    for(int idex = 0 ; idex < NUM_LEDS; idex++ ) {
      leds[idex] = CHSV(thishue, thissat, thisbright);
    }
}

void twinkle( CRGB* L ) {
  static byte huebase = 0;
  
  //slowly rotate huebase
  if( random8(4) == 0) huebase--;

  for( int whichPixel = 0; whichPixel < NUM_LEDS; whichPixel++) {  
    int hue = random8(32) + huebase;
    int saturation = 255;    // richest color
    int brightness = dim8_video( dim8_video( dim8_video( random8())));
    
    L[whichPixel] = CHSV( hue, saturation, brightness);
  }
}

void matrix() 
{
  int rand = random8(0, 100);
  if (rand > 90) {
    leds[0] = CHSV(random8(), thissat, thisbright);
  }
  else {
    leds[0] = CHSV(0, thissat, 0);
  }

  for(int i = NUM_LEDS-1; i > 0; i--) 
  {
    leds[i] = leds[i-1];
  }
}

void correct_color_balance()
{
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i].g = scale8_video( leds[i].g, 192);
  }
}


CHSV sv_ramp( uint8_t hue, uint8_t ramp) {
  uint8_t brightness;
  uint8_t saturation;
  if( ramp < 128) {
    // fade toward black
    brightness = ramp * 2;
    brightness = dim8_video( brightness);
//    uint8_t global_brightness = FastLED.getBrightness();
//    uint8_t min_step = 256 / global_brightness;
//    brightness = qadd8( min_step * 16, brightness);
    saturation = 255;
  } else {
    // desaturate toward white
    brightness = 255;
    saturation = 255 - ((ramp - 128) * 2);
    saturation = 255 - dim8_video( 255 - saturation);
  }
  return CHSV( hue, saturation, brightness); }

void loop5( CRGB* L )
{
  uint8_t GB = FastLED.getBrightness();
  uint8_t boost = 0;
//  if( GB < 65) boost += 8;
//  if( GB < 33) boost += 8;
  
  uint8_t N = 2;
  
  static uint16_t starttheta = 0;
  starttheta += 100 / N;

  static uint16_t starthue16 = 0;
  starthue16 += 20 / N;
  

  uint16_t hue16 = starthue16;
  uint16_t theta = starttheta;
  for( int i = 0; i < NUM_LEDS; i++) {
    uint8_t frac = (sin16( theta) + 32768) / 256;
    frac = scale8(frac,160) + 32;
    theta += 3700;

    hue16 += 2000;
    uint8_t hue = hue16 / 256;
    L[i] = sv_ramp( hue, frac + boost);
  }
}

//---FIND ADJACENT INDEX CLOCKWISE
int adjacent_cw(int i) {
  int r;
  if (i < NUM_LEDS - 1) {r = i + 1;}
  else {r = 0;}
  return r;
}

//---FIND ADJACENT INDEX COUNTER-CLOCKWISE
int adjacent_ccw(int i) {
  int r;
  if (i > 0) {r = i - 1;}
  else {r = NUM_LEDS - 1;}
  return r;
}

void random_march() 
{
  int iCCW;
  for(int idex = 0; idex < NUM_LEDS; idex++ ) {
    iCCW = adjacent_ccw(idex);
    leds[idex] = leds[idex+1];
  }
  leds[NUM_LEDS-1] = CHSV(random8(), thissat, thisbright);
}

void incrementState()
{
  state++;
  if(state > NUM_MODES){ 
    state = 1; 
  }
}

void setDemoMode()
{
    demoMode = 1;
    demoStateCountdown = DEMO_COUNT;
//    digitalWrite(LED2,HIGH);
}

void colorWaves()
{
  colorwaves( leds, NUM_LEDS, IceColors_p);
}

// ColorWavesWithPalettes by Mark Kriegsman: https://gist.github.com/kriegsman/8281905786e8b2632aeb
// This function draws color waves with an ever-changing,
// widely-varying set of parameters, using a color palette.
void colorwaves( CRGB* ledarray, uint16_t numleds, CRGBPalette16& palette)
{
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;

  // uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 300, 1500);

  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;

  for ( uint16_t i = 0 ; i < numleds; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;
    uint16_t h16_128 = hue16 >> 7;
    if ( h16_128 & 0x100) {
      hue8 = 255 - (h16_128 >> 1);
    } else {
      hue8 = h16_128 >> 1;
    }

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    uint8_t index = hue8;
    //index = triwave8( index);
    index = scale8( index, 240);

    CRGB newcolor = ColorFromPalette( palette, index, bri8);

    uint16_t pixelnumber = i;
    pixelnumber = (numleds - 1) - pixelnumber;

    nblend( ledarray[pixelnumber], newcolor, 128);
  }
}

void juggle()
{
  static uint8_t    numdots =   4; // Number of dots in use.
  static uint8_t   faderate =   2; // How long should the trails be. Very low value = longer trails.
  static uint8_t     hueinc =  255 / numdots - 1; // Incremental change in hue between each dot.
  static uint8_t    thishue =   0; // Starting hue.
  static uint8_t     curhue =   0; // The current hue
  static uint8_t    thissat = 255; // Saturation of the colour.
  static uint8_t thisbright = 255; // How bright should the LED/display be.
  static uint8_t   basebeat =   5; // Higher = faster movement.

  static uint8_t lastSecond =  99;  // Static variable, means it's only defined once. This is our 'debounce' variable.
  uint8_t secondHand = (millis() / 1000) % 30; // IMPORTANT!!! Change '30' to a different value to change duration of the loop.

  if (lastSecond != secondHand) { // Debounce to make sure we're not repeating an assignment.
    lastSecond = secondHand;
    switch (secondHand) {
      case  0: numdots = 1; basebeat = 20; hueinc = 16; faderate = 2; thishue = 0; break; // You can change values here, one at a time , or altogether.
      case 10: numdots = 4; basebeat = 10; hueinc = 16; faderate = 8; thishue = 128; break;
      case 20: numdots = 8; basebeat =  3; hueinc =  0; faderate = 8; thishue = random8(); break; // Only gets called once, and not continuously for the next several seconds. Therefore, no rainbows.
      case 30: break;
    }
  }

  // Several colored dots, weaving in and out of sync with each other
  curhue = thishue; // Reset the hue values.
  fadeToBlackBy(leds, NUM_LEDS, faderate);
  for ( int i = 0; i < numdots; i++) {
    //beat16 is a FastLED 3.1 function
    leds[beatsin16(basebeat + i + numdots, 0, NUM_LEDS)] += CHSV(gHue + curhue, thissat, thisbright);
    curhue += hueinc;
  }
}

void fire()
{
  heatMap(HeatColors_p, true);
}

void water()
{
  heatMap(IceColors_p, false);
}

// based on FastLED example Fire2012WithPalette: https://github.com/FastLED/FastLED/blob/master/examples/Fire2012WithPalette/Fire2012WithPalette.ino
void heatMap(CRGBPalette16 palette, bool up)
{
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy(random(256));

  // Array of temperature readings at each simulation cell
  static byte heat[256];

  byte colorindex;

  // Step 1.  Cool down every cell a little
  for ( uint16_t i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8( heat[i],  random8(0, ((cooling * 10) / NUM_LEDS) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for ( uint16_t k = NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if ( random8() < sparking ) {
    int y = random8(7);
    heat[y] = qadd8( heat[y], random8(160, 255) );
  }

  // Step 4.  Map from heat cells to LED colors
  for ( uint16_t j = 0; j < NUM_LEDS; j++) {
    // Scale the heat value from 0-255 down to 0-240
    // for best results with color palettes.
    colorindex = scale8(heat[j], 190);

    CRGB color = ColorFromPalette(palette, colorindex);

    if (up) {
      leds[j] = color;
    }
    else {
      leds[(NUM_LEDS - 1) - j] = color;
    }
  }
}

void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( uint8_t chanceOfGlitter)
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

// Pride2015 by Mark Kriegsman: https://gist.github.com/kriegsman/964de772d64c502760e5
// This function draws rainbows with an ever-changing,
// widely-varying set of parameters.
void pride()
{
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;

  uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 1, 3000);

  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;

  for ( uint16_t i = 0 ; i < NUM_LEDS; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    CRGB newcolor = CHSV( hue8, sat8, bri8);

    uint16_t pixelnumber = i;
    pixelnumber = (NUM_LEDS - 1) - pixelnumber;

    nblend( leds[pixelnumber], newcolor, 64);
  }
}

void candyCane() {
  for (uint8_t i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = ColorFromPalette(RedWhite_p, zCoords[i] - beat8(speed));
  }
}

void fallingPalette()
{
  for (uint8_t i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = ColorFromPalette(gCurrentPalette, zCoords[i] / 4 + beat8(speed));
  }
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(speed, 0, NUM_LEDS);
  static int prevpos = 0;
  if( pos < prevpos ) {
    fill_solid( leds+pos, (prevpos-pos)+1, CHSV(gHue,220,255));
  } else {
    fill_solid( leds+prevpos, (pos-prevpos)+1, CHSV( gHue,220,255));
  }
  prevpos = pos;
}

byte alt = 0;
void loop() {
  if (LIGHT_TEST)
  {
    lightTest();
    return;
  }
  
  demoStateCountdown = (demoStateCountdown < 0) ? 0 : demoStateCountdown;

  if (demoMode)
  {
    demoStateCountdown--;
    if (demoStateCountdown <= 0)
    {
      incrementState();
      demoStateCountdown = DEMO_COUNT;
    }
  }
  
  int adjdelay = thisdelay;
  
  switch (state)
  {
    case 1:
      //adjdelay = (adjdelay < 120) ? 120 : adjdelay;
      //fallingPalette();
      random_burst();
    break;
    case 2:
      rainbowWithGlitter();
    break;
    case 3:
      rainbow_loop();
    break;
    case 4:
      rgb_propeller();
    break;
    case 5:
      rainbow();
    break;
    case 6:
      rainbow_fade();
      if ((demoMode) && !(demoStateCountdown%5)) demoStateCountdown-=2;
      adjdelay = (adjdelay < 30) ? 30 : adjdelay;
    break;
    case 7:
      pride();
    break;
    case 8:
      if ((demoMode) && !(demoStateCountdown%5)) demoStateCountdown-=2;
      adjdelay = (adjdelay < 50) ? 50 : adjdelay;
      matrix();
    break;
    case 9:
      demoStateCountdown -= 2;
      adjdelay = (adjdelay < 120) ? 120 : adjdelay;
      random_march();
    break;
    case 10:
      loop5(leds);
      adjdelay = adjdelay/3;
    break;
    case 11:
      twinkle(leds);
    break;
    case 12:
      fallingPalette();
    break;
    case 13:
      juggle();
    break;
    case 14:
      colorWaves();
    break;
    case 15:
      water();
    break;
    case 16:
      fire();
    break;
    case 17:
      candyCane();
    break;
    case 18:
      candycane();
    break;
    case 19:
      sinelon();
    break;
    case 20:
      if (!USE_LEVEL_ANIMATIONS) incrementState();
      adjdelay = (adjdelay < 100) ? 100 : adjdelay;
      demoStateCountdown -= 15;
      random_levels();
    break;
    case 21:
      if (!USE_LEVEL_ANIMATIONS) incrementState();
      rainbow();
      adjdelay = (adjdelay < 50) ? 50 : adjdelay;
      if (demoMode) demoStateCountdown -= 600;
      drain();
      pancake();
    break;
    case 22:
      confetti();
    break;
   default:
      random_burst();
    break;
  }
  FastLED.show();  
  delay(adjdelay);
}
