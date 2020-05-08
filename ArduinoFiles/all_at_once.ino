/* Simple music visualiser - Arduino side.
 * Takes input data from the serial port (converted from sound data on the PC side),
 * converts it to a colour on the HSV wheel and sends it to the LED array, 
 * using the FastLED library. New data affects each pin at the same time. This one
 * is worse than middle_outward.ino IMO.
 */

#include <FastLED.h>
#define DATA_PIN 9
#define CLOCK_PIN 8

CRGB leds[50];

void setup() {
  Serial.begin(19200);
  FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, 50);
  for (int i = 0; i < 50; i++) //Initialise LED array as all red
  {
    leds[i].setHue(0);
  }
  FastLED.show();
}

int check = 0;
byte input;

void loop() {
  if (Serial.available())
  {
    input = Serial.read();
    if (check < 2) //the computer will send a couple bytes
    {           //when connecting to the Arduino
      check++;
    }
    else
    {
      for(int i = 0; i < 50; i++)
      {
        leds[i].setHue((abs(i-25)*5 + input) % 256);
      }
    }
    FastLED.show();
  }
}
