/*------------------------------------------------------------------------------
  23/20/2020
  MAX9814 code from https://github.com/acrobotic/Ai_Demos_ESP32/blob/master/vu_meter/vu_meter.ino
  
------------------------------------------------------------------------------*/
#include<FastLED.h>
#include<MegunoLink.h>
#include<Filter.h>

// define necessary parameters
#define MIC_PIN   A0

#define NOISE 780
#define LED_PIN     5
#define NUM_LEDS    204 // 144 + 60 LEDs on strip
//#define TOP   (NUM_LEDS+2) // allow the max level to be slightly off scale
#define TOP 255
#define BRIGHTNESS  50
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
unsigned long lastUpdate;

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

int randNumber;

#define SCROLL_SPEED 20
#define UPDATES_PER_SECOND 100

// define the variables needed for the audio levels
int lvl = 0, minLvl = 0, maxLvl = 50; // tweak the min and max as needed

// instantiate the filter class for smoothing the raw audio signal
ExponentialFilter<long> ADCFilter(5,0);

void setup() {
  // put your setup code here, to run once:
  delay( 3000 ); // power-up safety delay
  Serial.begin(9600);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );

  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;

  randomSeed(analogRead(0));
/*  // set all LED dark at the beginning
  for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Black;
  } */
  
}

unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 10000;           // interval at which to blink (milliseconds)

void loop() {
  uint8_t sec_ = (millis() / 1000) % 90;
  if (sec_<=30){pulse();}
  if ((sec_>30)&&(sec_<60)){mixed();}
  if (sec_>60){showlevel();}

  //showlevel();
}


int soundlevel(){
  // put your main code here, to run repeatedly:
  // read the audio signal and filter it
  int n, height;
  n = analogRead(MIC_PIN);
  // remove the MX9814 bias of 1.25VDC
  n = abs(1023 - n);
  // hard limit noise/hum
  n = (n <= NOISE) ? 0 : abs(n - NOISE);
  // apply the exponential filter to smooth the raw signal
  ADCFilter.Filter(n);
  lvl = ADCFilter.Current();
//  // plot the raw versus filtered signals
//  Serial.print(n);
//  Serial.print(" ");
//  Serial.println(lvl);
  height = TOP * (lvl - minLvl) / (long)(maxLvl - minLvl);
  if(height < 0L) height = 0;
  else if(height > TOP) height = TOP;
 
  return height;
}

void pulse(){
  int height;
  uint8_t brightness = 255;
  height=soundlevel();
  if (height>10){ 
    leds[0]=ColorFromPalette(currentPalette, height, brightness, currentBlending);
  }
  else{
    leds[0]=CRGB::Black;
  }

  if (millis() - lastUpdate > SCROLL_SPEED){
    lastUpdate += SCROLL_SPEED;
    for (int i= NUM_LEDS -1; i>0;i--){
      leds[i]=leds[i-1];      
    }
    FastLED.show();
  }
}

void showlevel(){
  int height;
  height=soundlevel();
  // turn the LEDs corresponding to the level on/off
  for(uint8_t i = 0; i < NUM_LEDS; i++) {
    // turn off LEDs above the current level
    if(i >= height) leds[i] = CRGB(0,0,0);
    // otherwise, turn them on!
    else leds[i] = Wheel( map( i, 0, NUM_LEDS-1, 30, 150 ) );
  }
  FastLED.show();
}


void mixed(){
    ChangePalettePeriodically();
    
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */
    
    FillLEDsFromPaletteColors( startIndex);
    //Serial.println(startIndex);
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);
}


CRGB Wheel(byte WheelPos) {
  // return a color value based on an input value between 0 and 255
  if(WheelPos < 85)
    return CRGB(WheelPos * 3, 255 - WheelPos * 3, 0);
  else if(WheelPos < 170) {
    WheelPos -= 85;
    return CRGB(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return CRGB(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}


void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;

    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }

}

void ChangePalettePeriodically()
{
    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;
    
    if( lastSecond != secondHand) {
        lastSecond = secondHand;
        randNumber=random(11);
        //Serial.println(randNumber);
        if (secondHand % 10 == 0){
          if( randNumber ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
          if( randNumber ==  1)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }
          if( randNumber ==  2)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
          if( randNumber ==  3)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
          if( randNumber ==  4)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND; }
          if( randNumber ==  5)  { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND; }
          if( randNumber ==  6)  { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND; }
          if( randNumber ==  7)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
          if( randNumber ==  8)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
          if( randNumber ==  9)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;  }
          if( randNumber ==  10)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }   
        }
    }
}



// This function fills the palette with totally random colors.
void SetupTotallyRandomPalette()
{
    for( int i = 0; i < 16; i++) {
        currentPalette[i] = CHSV( random8(), 255, random8());
    }
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;
    
}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    currentPalette = CRGBPalette16(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}


// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};
