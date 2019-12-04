
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include <IRremote.h>
#include "define.h"
#include "FastLED.h"
#include "ArduinoJson.h"
#include "StripLED.h"
#include "StripCommand.h"
#include "IRdefine.h"
#include "Thunder.h"


StripCommand StripCommander;

Thunder T1 = Thunder("THUNDER.WAV", &StripCommander);
Thunder T2 = Thunder("THUNDER.WAV", &StripCommander);

elapsedMillis RefreshOutputTimer = 0;
#define OUTPUT_REFRESH_RATE 7
elapsedMillis IRRepeatTimeout = 0;
#define IR_REPEAT_TIMEOUT 200

boolean LastFrameShowed = true;

DynamicJsonBuffer jsonBuffer;
char input[100];


// GUItool: begin automatically generated code
AudioPlaySdWav           Rain;     //xy=102,155
AudioPlaySdWav           ThunderLeft;     //xy=112,285
AudioPlaySdWav           ThunderRight;     //xy=114,336
AudioPlaySdWav           ThunderCenter;     //xy=115,401
AudioMixer4              mixer1;         //xy=366,222
AudioMixer4              mixer2;         //xy=367,300
AudioOutputI2S           i2s1;           //xy=569,260
AudioConnection          patchCord1(Rain, 0, mixer1, 0);
AudioConnection          patchCord2(Rain, 1, mixer2, 0);
AudioConnection          patchCord3(ThunderLeft, 0, mixer1, 1);
AudioConnection          patchCord4(ThunderLeft, 1, mixer1, 2);
AudioConnection          patchCord5(ThunderCenter, 0, mixer1, 3);
AudioConnection          patchCord6(ThunderCenter, 1, mixer2, 3);
AudioConnection          patchCord7(ThunderRight, 0, mixer2, 1);
AudioConnection          patchCord8(ThunderRight, 1, mixer2, 2);
AudioConnection          patchCord9(mixer1, 0, i2s1, 0);
AudioConnection          patchCord10(mixer2, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=424,498
// GUItool: end automatically generated code
// Use these with the audio adaptor board
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

//// Prototypes ////
void serialParse();
void init_Player();
ircmd getIRCmd();
void IR_Management();
void taskManager();
void toggleRain();
void startTHunder();


uint8_t Brightness = 0;

void setup()
{
  FastLED.addLeds<NEOPIXEL, 2>(StripCommander.leds, 0, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, 3>(StripCommander.leds, NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, 4>(StripCommander.leds, 2 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, 5>(StripCommander.leds, 3 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);


  Serial.begin(115200);
#ifdef DEBUG_MODE
  while (!Serial);
#endif

#ifdef DEBUG_MODE
  Serial.println("Port Opened.");
  Serial.println("Initializing");
#endif
  StripCommander.begin();
  IR.enableIRIn();
  init_Player();

  T1.addEvent(100, SingleFlash);
  T1.addEvent(600, GroupFlash);
  T1.addEvent(800, GroupFlash);

#ifdef DEBUG_MODE
  Serial.println("Successfully Initialized.");
#endif
  FastLED.show();   //Clear all LEDs
}

void loop()
{
  taskManager();
}

void taskManager()
{
  IR_Management();

  serialParse();

  if (RefreshOutputTimer >= OUTPUT_REFRESH_RATE)
  {
    RefreshOutputTimer = 0;
    if (StripCommander.RunningFX)  //Update output only if necessary, RefreshOutputTimer is reset only if frame is displayed ==> As soon as one frame il
    {
      StripCommander.StateChanged = false;
      FastLED.show();
    }
    else if (!LastFrameShowed)  //Show one more frame after RunningFX has been reset to be sure last effect iteration has been showed.
    {
      LastFrameShowed = true;
      FastLED.show();
    }
    StripCommander.dynamicStateUpdate();
  }
}

void IR_Management()
{
  switch (getIRCmd())
  {
    case Ip:
      Serial.println("Intensity +");
      Brightness  = constrain(Brightness + 10, 0, 255);
      IRRepeatTimeout = 0;
      LastIRCmd = Ip;
      break;

    case Im:
      Serial.println("Intensity -");
      Brightness  = constrain(Brightness - 10, 0, 255);
      IRRepeatTimeout = 0;
      LastIRCmd = Im;
      break;

    case Play:
        toggleRain();
      break;

    case Ca:
      StripCommander.fadeToHSV(10, 255, 255, 1500);
      break;

    case Flash:
        Serial.println("Start Thunder");
        startTHunder();
      break;

    case RPT:
      Serial.println("Repeat");
      if ( IRRepeatTimeout < IR_REPEAT_TIMEOUT)
      {
        switch (LastIRCmd)
        {
          case Ip:
            Brightness  = constrain(Brightness + 1, 0, 255);
            break;
          case Im:
            Brightness  = constrain(Brightness - 1, 0, 255);
            break;
          default: break;
        }
        IRRepeatTimeout = 0;
      }
      else LastIRCmd = NO_CMD;
      break;

    default: break;
  }
}

ircmd getIRCmd()
{
  if ( IR.decode(&RawIRCmd))
  {
    if ((RawIRCmd.value == 0xFFFFFFFF))
    {
      IR.resume();
      return RPT;
    }
    else
    {
      for (int i = 0 ; i < NB_IR_CMD ; i++)
      {
        //Serial.println(RawIRCmd.value & 0x00FFFF, HEX);
        if ((RawIRCmd.value & 0x00FFFF) == IRcmdCommands[i])
        {
          IR.resume();
          return (ircmd)i;
        }
      }
      IR.resume();
    }
  }
  return NO_CMD;
}


void init_Player()
{
  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(8);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);

  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here, but print a message repetitively
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
}



void serialParse()
{
  if (Serial.available()) {
    char lastChar = '\0';
    int i = 0;

    while (lastChar != '\r') {
      if (Serial.available()) {
        lastChar = Serial.read();
        input[i] = lastChar;
        i++;
      }
    }
    while (Serial.available()) char a = Serial.read();

    JsonObject& root = jsonBuffer.parseObject(input);
    const char* method = root["method"];
    Serial.println(method);
    if (strcmp(method, "fadeToRGB") == 0)
    {
      StripCommander.fadeToRGB(root["R"], root["G"], root["B"], root["Delay"]);
    }
    else if (strcmp(method, "fadeToHSV") == 0) //{method:fadeToHSV,H:160,S:255,V:255,Delay:2000}
    {
      StripCommander.fadeToHSV(root["H"], root["S"], root["V"], root["Delay"]);
    }
    else if (strcmp(method, "setToRGB") == 0) //{method:setToRGB,R:255,G:100,B:0}
    {
      Serial.println("set to RGB value");
      StripCommander.setToRGB(root["R"], root["G"], root["B"]);
      LastFrameShowed = false;
    }
    else if (strcmp(method, "setToHSV") == 0) //{method:setToHSV,H:255,S:100,V:0}
    {
      Serial.println("set to HSV value");
      StripCommander.setToHSV(root["H"], root["S"], root["V"]);
      LastFrameShowed = false;
    }
    else if (strcmp(method, "rainbow") == 0)    //{method:rainbow}
    {
      Serial.println("Rainbow");
      StripCommander.rainbow();
      LastFrameShowed = false;
    }
    else if (strcmp(method, "flash") == 0)
    {
      Serial.println("Flash");      //{method:flash}
      uint8_t FlashCount = random(1, 15);
      for (uint8_t i = 0; i <= FlashCount; i++)
      {
        StripCommander.flash(random(0, NUM_LEDS));
      }
    }
    else if (strcmp(method, "groupFlash") == 0) //{method:groupFlash,Group:0,Direction:1}
    {
      StripCommander.groupFlash(root["Group"], root["Direction"]);
    }
    else if (strcmp(method, "rdmGroupFlash") == 0) //{method:rdmGroupFlash}
    {
      StripCommander.groupFlash();
    }
    else if (strcmp(method, "rain") == 0) //{method:rain}
    {
      toggleRain();
    }
    else if (strcmp(method, "thunder") == 0) //{method:thunder}
    {
      startTHunder();
    }
    else
    {
      Serial.println("ca ne marche pas ^^");
    }
  }
}


void toggleRain()
{
      if (!Rain.isPlaying())
      {
        Rain.play("RAIN.WAV");
      }
      else
      {
        Rain.stop();
      }
}

void startTHunder()
{
  T1.trig(&ThunderCenter);
}