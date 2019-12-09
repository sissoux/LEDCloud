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

//Declare an array of pointers to the thunder objects for random selection
#define MAX_NUMBER_OF_THUNDERS 30
Thunder *Thunders[MAX_NUMBER_OF_THUNDERS];
uint8_t NumberOfInitializedThunders = 0;
#define NUMBER_OF_PLAYER 3

elapsedMillis RefreshOutputTimer = 0;
#define OUTPUT_REFRESH_RATE 7
elapsedMillis IRRepeatTimeout = 0;
#define IR_REPEAT_TIMEOUT 200

boolean LastFrameShowed = true;

char input[100];

// GUItool: begin automatically generated code
AudioPlaySdWav Rain;          //xy=102,155
AudioPlaySdWav ThunderLeft;   //xy=112,285
AudioPlaySdWav ThunderRight;  //xy=114,336
AudioPlaySdWav ThunderCenter; //xy=115,401
AudioPlaySdWav *ThunderPLayers[NUMBER_OF_PLAYER] = {&ThunderRight, &ThunderLeft, &ThunderCenter};
AudioMixer4 mixer1;  //xy=366,222
AudioMixer4 mixer2;  //xy=367,300
AudioOutputI2S i2s1; //xy=569,260
AudioConnection patchCord1(Rain, 0, mixer1, 0);
AudioConnection patchCord2(Rain, 1, mixer2, 0);
AudioConnection patchCord3(ThunderLeft, 0, mixer1, 1);
AudioConnection patchCord4(ThunderLeft, 1, mixer1, 2);
AudioConnection patchCord5(ThunderCenter, 0, mixer1, 3);
AudioConnection patchCord6(ThunderCenter, 1, mixer2, 3);
AudioConnection patchCord7(ThunderRight, 0, mixer2, 1);
AudioConnection patchCord8(ThunderRight, 1, mixer2, 2);
AudioConnection patchCord9(mixer1, 0, i2s1, 0);
AudioConnection patchCord10(mixer2, 0, i2s1, 1);
AudioControlSGTL5000 sgtl5000_1; //xy=424,498
// GUItool: end automatically generated code
// Use these with the audio adaptor board
#define SDCARD_CS_PIN 10
#define SDCARD_MOSI_PIN 7
#define SDCARD_SCK_PIN 14

//// Prototypes ////
void serialParse();
void init_Player();
ircmd getIRCmd();
void IR_Management();
void taskManager();
void toggleRain();
void startRandomThunder();
void initThunders();

uint8_t Brightness = 0;

void setup()
{
  FastLED.addLeds<NEOPIXEL, 2>(StripCommander.leds, 0, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, 3>(StripCommander.leds, NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, 4>(StripCommander.leds, 2 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, 5>(StripCommander.leds, 3 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  initThunders();
  Serial.begin(115200);
#ifdef DEBUG_MODE
  while (!Serial)
    ;
#endif

#ifdef DEBUG_MODE
  Serial.println("Port Opened.");
  Serial.println("Initializing");
#endif
  StripCommander.begin();
  IR.enableIRIn();
  init_Player();

#ifdef DEBUG_MODE
  Serial.println("Successfully Initialized.");
#endif
  FastLED.show(); //Clear all LEDs
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
    for (int T = 0; T < NumberOfInitializedThunders; T++)
    {
      Thunders[T]->Update();
    }
    if (StripCommander.RunningFX) //Update output only if necessary, RefreshOutputTimer is reset only if frame is displayed ==> As soon as one frame il
    {
      StripCommander.StateChanged = false;
      FastLED.show();
    }
    else if (!LastFrameShowed) //Show one more frame after RunningFX has been reset to be sure last effect iteration has been showed.
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
    Brightness = constrain(Brightness + 10, 0, 255);
    IRRepeatTimeout = 0;
    LastIRCmd = Ip;
    break;

  case Im:
    Serial.println("Intensity -");
    Brightness = constrain(Brightness - 10, 0, 255);
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
    startRandomThunder();
    break;

  case RPT:
    Serial.println("Repeat");
    if (IRRepeatTimeout < IR_REPEAT_TIMEOUT)
    {
      switch (LastIRCmd)
      {
      case Ip:
        Brightness = constrain(Brightness + 1, 0, 255);
        break;
      case Im:
        Brightness = constrain(Brightness - 1, 0, 255);
        break;
      default:
        break;
      }
      IRRepeatTimeout = 0;
    }
    else
      LastIRCmd = NO_CMD;
    break;

  default:
    break;
  }
}

ircmd getIRCmd()
{
  if (IR.decode(&RawIRCmd))
  {
    if ((RawIRCmd.value == 0xFFFFFFFF))
    {
      IR.resume();
      return RPT;
    }
    else
    {
      for (int i = 0; i < NB_IR_CMD; i++)
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
  if (!(SD.begin(SDCARD_CS_PIN)))
  {
    // stop here, but print a message repetitively
    while (1)
    {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
}

void serialParse()
{
  if (Serial.available())
  {
    char lastChar = '\0';
    int i = 0;

    while (lastChar != '\r')
    {
      if (Serial.available())
      {
        lastChar = Serial.read();
        input[i] = lastChar;
        i++;
      }
    }
    while (Serial.available())
      Serial.read();

    DynamicJsonDocument SerialJsonDoc(1024); //Arbritrary set to 1024, need to evaluate datacontracts to check the size
    deserializeJson(SerialJsonDoc, input);

    const char *method = SerialJsonDoc["method"];
    Serial.println(method);
    if (strcmp(method, "fadeToRGB") == 0)
    {
      StripCommander.fadeToRGB(SerialJsonDoc["R"], SerialJsonDoc["G"], SerialJsonDoc["B"], SerialJsonDoc["Delay"]);
    }
    else if (strcmp(method, "fadeToHSV") == 0) //{method:fadeToHSV,H:160,S:255,V:255,Delay:2000}
    {
      StripCommander.fadeToHSV(SerialJsonDoc["H"], SerialJsonDoc["S"], SerialJsonDoc["V"], SerialJsonDoc["Delay"]);
    }
    else if (strcmp(method, "setToRGB") == 0) //{method:setToRGB,R:255,G:100,B:0}
    {
      Serial.println("set to RGB value");
      StripCommander.setToRGB(SerialJsonDoc["R"], SerialJsonDoc["G"], SerialJsonDoc["B"]);
      LastFrameShowed = false;
    }
    else if (strcmp(method, "setToHSV") == 0) //{method:setToHSV,H:255,S:100,V:0}
    {
      Serial.println("set to HSV value");
      StripCommander.setToHSV(SerialJsonDoc["H"], SerialJsonDoc["S"], SerialJsonDoc["V"]);
      LastFrameShowed = false;
    }
    else if (strcmp(method, "rainbow") == 0) //{method:rainbow}
    {
      Serial.println("Rainbow");
      StripCommander.rainbow();
      LastFrameShowed = false;
    }
    else if (strcmp(method, "flash") == 0)
    {
      Serial.println("Flash"); //{method:flash}
      uint8_t FlashCount = random(1, 15);
      for (uint8_t i = 0; i <= FlashCount; i++)
      {
        StripCommander.flash(random(0, NUM_LEDS));
      }
    }
    else if (strcmp(method, "groupFlash") == 0) //{method:groupFlash,Group:0,Direction:1}
    {
      StripCommander.groupFlash(SerialJsonDoc["Group"], SerialJsonDoc["Direction"]);
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
      Serial.print("Trigering Thunder");
      startRandomThunder();
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

void startRandomThunder()
{
  uint8_t player = random8(3);
  AudioPlaySdWav *playerPointer = nullptr;
  //Select first available player starting from random position
  for (uint8_t PlayerCounter = 0; PlayerCounter < NUMBER_OF_PLAYER; PlayerCounter++)
  {
    uint8_t SelectedPlayer = (PlayerCounter + player) % NUMBER_OF_PLAYER;
    if (!ThunderPLayers[SelectedPlayer]->isPlaying())
    {
      playerPointer = ThunderPLayers[SelectedPlayer];
      break;
    }
  }
  //Only trig thunder if we found an available player
  if (playerPointer != nullptr)
    Thunders[random(0, NumberOfInitializedThunders)]->trig(playerPointer);
}

void initThunders(Thunder *P_Thunder[])
{
  //TODO : read thunder events from a file in SD Card (use JSON)
  //File myFile = SD.open("test.txt", FILE_WRITE);
  for (int i = 0; i < MAX_NUMBER_OF_THUNDERS; i++)
  {
    P_Thunder[i] = nullptr;
  }
  P_Thunder[0] = new Thunder("HEAVY1.wav", &StripCommander, Heavy);
  P_Thunder[1] = new Thunder("HEAVY2.wav", &StripCommander, Heavy);
  P_Thunder[2] = new Thunder("HEAVY3.wav", &StripCommander, Heavy);
  P_Thunder[3] = new Thunder("HEAVY4.wav", &StripCommander, Heavy);
  P_Thunder[4] = new Thunder("MEDIUM1.wav", &StripCommander, Medium);
  P_Thunder[5] = new Thunder("MEDIUM2.wav", &StripCommander, Medium);
  P_Thunder[6] = new Thunder("MEDIUM3.wav", &StripCommander, Medium);
  P_Thunder[7] = new Thunder("DISTANT1.wav", &StripCommander, Distant);
  P_Thunder[8] = new Thunder("DISTANT2.wav", &StripCommander, Distant);
  P_Thunder[9] = new Thunder("DISTANT3.wav", &StripCommander, Distant);
  P_Thunder[10] = new Thunder("DISTANT4.wav", &StripCommander, Distant);
  P_Thunder[11] = new Thunder("DISTANT5.wav", &StripCommander, Distant);
  P_Thunder[12] = new Thunder("VHEAVY1.wav", &StripCommander, VeryHeavy);
  NumberOfInitializedThunders = 13;
/*
  P_Thunder[0]->addEvent((Event[]){{100, GroupFlash}, {200, GroupFlash}, {200, GroupFlash}}, 3);
  P_Thunder[1]->addEvent((Event[]){{100, SingleFlash}, {200, SingleFlash}, {600, GroupFlash}}, 3);
  P_Thunder[2]->addEvent(100, BigFlash);
  P_Thunder[3]->addEvent((Event[]){{500, SingleFlash}, {1800, GroupFlash}, {2000, GroupFlash}}, 3);
  
  M1.addEvent(400, BigFlash);
  M1.addEvent(2500, GroupFlash);
  M1.addEvent(3000, BigFlash);

  M2.addEvent(500, SingleFlash);
  M2.addEvent(900, BigFlash);

  M3.addEvent(100, BigFlash);
  M3.addEvent(600, GroupFlash);

  D1.addEvent(1000, SingleFlash);
  D1.addEvent(2500, GroupFlash);
  D1.addEvent(2800, BigFlash);

  D2.addEvent(100, SingleFlash);

  D3.addEvent(2000, SingleFlash);
  D3.addEvent(3000, GroupFlash);

  D4.addEvent(17000, SingleFlash);
  D4.addEvent(1800, SingleFlash);
  D4.addEvent(20000, GroupFlash);
  D4.addEvent(21000, BigFlash);

  D5.addEvent(0, BigFlash);

  VH.addEvent(800, SingleFlash);
  VH.addEvent(1100, MegaFlash);
  VH.addEvent(1500, SingleFlash);
  VH.addEvent(1800, SingleFlash);
  VH.addEvent(2000, SingleFlash);
  VH.addEvent(3500, SingleFlash);
*/
}
