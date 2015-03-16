
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
unsigned int ChainDelay = 1;
unsigned long Time = 0;
unsigned long TIMEU = 0;
LED LED[NUMPIXELS];
pixelState State = FLASH;

int Chain = 0;
int Index = 0;

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 1000; // delay for half a second

void setup() 
{
  pixels.begin(); // This initializes the NeoPixel library.
  Serial.begin(115200);

}

void loop() 
{
  if (Serial.available())
  {
    char data = Serial.read();
    switch (data)
    {
    case 'f':
      {
        int x = random(0,NUMPIXELS-1);
        LED[x].Trig = 1;
        LED[x].Tf = random(50,300);
        Delay[x] = random(FLASH_DELAY_L,FLASH_DELAY_H);
      }
      break;

    case 'c':
      Chain = 1;
      Index = 0;
      break;

    default:
      break;
    }
  }
  if (millis() - TIMEU > delayval)
  {
    TIMEU = millis();
    Chain = 1;
    Index = 0;
  }

  if (Chain)
  {
    if (!LED[Index].Active)
    {
      LED[Index].Trig = 1;
      LED[Index].Tf = random(2,100);
    }
    //Serial.println(Index);
    if (millis() - Time > ChainDelay)
    {
      ChainDelay = random(0,10);
      Time = millis();
      if (Index < NUMPIXELS)
      {
        Index++;
      }
      else if (!LED[Index].Active)
      {
        Index = 0;
        Chain = 0;
      }
    }
  }

  for (int x = 0; x < NUMPIXELS; x++)
  {
    LED[x].update(State);
    pixels.setPixelColor(x,pixels.Color(LED[x].R, LED[x].G, LED[x].B));
  }
  pixels.show();

}










