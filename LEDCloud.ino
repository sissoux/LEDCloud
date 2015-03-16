
#include <Adafruit_NeoPixel.h>
#include "LED.h"

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN            2

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      47
#define FLASH_DELAY_H 20000
#define FLASH_DELAY_L 500

unsigned long Temps[NUMPIXELS];
unsigned int Delay[NUMPIXELS];
LED LED[NUMPIXELS];

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 500; // delay for half a second

void setup() 
{
  pixels.begin(); // This initializes the NeoPixel library.
}

void loop() {
  
  for (int x = 0; x < NUMPIXELS; x++)
  {
    if (millis() - Temps[x] > Delay[x])
    {
      Temps[x] = millis();
      LED[x].Trig = 1;
      LED[x].Tf = random(50,300);
      Delay[x] = random(FLASH_DELAY_L,FLASH_DELAY_H);
    }
    LED[x].update();
    pixels.setPixelColor(x,pixels.Color(LED[x].R, LED[x].G, LED[x].B));
  }
  pixels.show();
  
}


  
