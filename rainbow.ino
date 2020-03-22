#include <FastLED.h>
#define NUM_LEDS 50
#define DATA_PIN 9
#define CLOCK_PIN 8

CRGB leds[NUM_LEDS];

void setup() {
   FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
   for (int i = 0; i < NUM_LEDS; i++)
   {
     leds[i].setHue(0);
   }
   FastLED.show();
}

int j = 0;
void loop() {
   for (int i = 0; i < NUM_LEDS; i++)
   {
     leds[i].setHue(j+(abs(i-25)*5));
   }    
   j++;
   if (j == 256)
   {
     j = 0;
   }
   FastLED.show();
   delay(5);
}
