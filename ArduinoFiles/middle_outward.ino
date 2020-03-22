/* Simple music visualiser - Arduino side.
 * Takes input data from the serial port (converted from sound data on the PC side),
 * converts it to a colour on the HSV wheel and sends it to the LED array, 
 * using the FastLED library. New data starts in the center and is pushed to the edges,
 * creating a ripple effect!
 */


#include <FastLED.h>

#define DATA_PIN 9
#define CLOCK_PIN 8

CRGB leds[50];

void setup() {
  Serial.begin(19200); //Begin receiving data from port
  FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS); //Set up the connection to the LED array
  for (int i = 0; i < 50; i++)
  {
    leds[i].setHue(0); //Initialise LED array, setting all the lights to red
  }
  FastLED.show();
}

byte input;

void loop() {
  if (Serial.available()) //When data is sent over the serial port
  {
    input = Serial.read();
    for(int i = 0; i < 24; i++)
    { //Colours get pushed towards the left edge on the left side,
      leds[i] = leds[i + 1];
    } //and to the right edge on the right side,
    for(int i = 49; i > 25; i--)
    {
      leds[i] = leds[i - 1];
    }
    leds[24].setHue(input); //and the new bytes define the center colour.
    leds[25].setHue(input);
    FastLED.show();
  }
}
