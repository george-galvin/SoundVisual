#include <FastLED.h>
#define DATA_PIN 9
#define CLOCK_PIN 8

CRGB leds[50];

void setup() {
  Serial.begin(19200);
  FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, 50);
  for (int i = 0; i < 50; i++)
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
    if (check < 2)
    {
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
