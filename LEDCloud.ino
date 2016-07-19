// BOF preprocessor bug prevent - insert me on top of your arduino-code
// From: http://www.a-control.de/arduino-fehler/?lang=en
#if 1
__asm volatile ("nop");
#endif

#include <Adafruit_NeoPixel.h>
#include "LED.h"
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

// GUItool: begin automatically generated code
AudioPlaySdWav           rainPlayer;       //xy=314,123
AudioPlaySdWav           thunderPlayer;       //xy=315,170
AudioMixer4              mixer1;         //xy=549,127
AudioMixer4              mixer2;         //xy=550,196
AudioOutputI2S           i2s1;           //xy=750,160
AudioConnection          patchCord1(rainPlayer, 0, mixer1, 0);
AudioConnection          patchCord2(rainPlayer, 1, mixer2, 0);
AudioConnection          patchCord3(thunderPlayer, 0, mixer1, 1);
AudioConnection          patchCord4(thunderPlayer, 1, mixer2, 1);
AudioConnection          patchCord5(mixer1, 0, i2s1, 0);
AudioConnection          patchCord6(mixer2, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=471,410
// GUItool: end automatically generated code


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

int T = 0;

int Chain = 0;
int Index = 0;

typedef enum{
  STBY,
  STORM
}
mode;

mode Mode = STBY;
mode serialParse();

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 1000; // delay for half a second

void setup() 
{
  pixels.begin(); // This initializes the NeoPixel library.
  Serial.begin(115200);
  while(!Serial);

  AudioMemory(5);

  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);

  SPI.setMOSI(7);
  SPI.setSCK(14);
  if (!(SD.begin(10))) {
    // stop here, but print a message repetitively
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
  Serial.println("Welcome to LEDCLOUD");

}

void loop() 
{
  Mode = serialParse();
  switch (Mode)
  {
  case STBY:
    State = BLACKOUT;
    if (rainPlayer.isPlaying())
    {
      rainPlayer.stop();
    }
    break;

  case STORM:
    State = FLASH;
    if (!rainPlayer.isPlaying())
    {
      rainPlayer.play("RAIN.WAV");
    }
    break;
  }


  if (T)
  {
    T = 0;
    int x = random(0,NUMPIXELS-1);
    LED[x].Trig = 1;
    LED[x].Tf = random(50,300);
    Delay[x] = random(FLASH_DELAY_L,FLASH_DELAY_H);
  }



  updateOutput();

}


void updateOutput()
{
  for (int x = 0; x < NUMPIXELS; x++)  // Update state of each pixel
  {
    LED[x].update(State);
    pixels.setPixelColor(x,pixels.Color(LED[x].R, LED[x].G, LED[x].B));
  }
  pixels.show();  //Show all pixels

}

mode serialParse()
{
  if (Serial.available())
  {
    char data = Serial.read();
    switch (data)
    {
    case 'f':
      Serial.println("FLASH !");
      T = 1;
      break;

    case 's':
      Serial.println("STORM !");
      return STORM;
      break;

    default:
      Serial.println("STBY !");
      return STBY;
      break;
    }
  }
  return Mode;
}














