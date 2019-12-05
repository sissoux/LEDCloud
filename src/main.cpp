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

//Declare all thunder objects
Thunder H1 = Thunder("HEAVY1.wav", &StripCommander);
Thunder H2 = Thunder("HEAVY2.wav", &StripCommander);
Thunder H3 = Thunder("HEAVY3.wav", &StripCommander);
Thunder H4 = Thunder("HEAVY4.wav", &StripCommander);
Thunder M1 = Thunder("MEDIUM1.wav", &StripCommander);
Thunder M2 = Thunder("MEDIUM2.wav", &StripCommander);
Thunder M3 = Thunder("MEDIUM3.wav", &StripCommander);
Thunder D1 = Thunder("DISTANT1.wav", &StripCommander);
Thunder D2 = Thunder("DISTANT2.wav", &StripCommander);
Thunder D3 = Thunder("DISTANT3.wav", &StripCommander);
Thunder D4 = Thunder("DISTANT4.wav", &StripCommander);
Thunder D5 = Thunder("DISTANT5.wav", &StripCommander);
Thunder VH = Thunder("VHEAVY1.wav", &StripCommander);

//Declare an array of pointers to the thunder objects for random selection
#define NUMBER_OF_THUNDERS 13
Thunder *Thunders[NUMBER_OF_THUNDERS] = {&H1, &H2, &H3, &H4, &M1, &M2, &M3, &D1, &D2, &D3, &D4, &D5, &VH};
#define NUMBER_OF_PLAYER 3

elapsedMillis RefreshOutputTimer = 0;
#define OUTPUT_REFRESH_RATE 7
elapsedMillis IRRepeatTimeout = 0;
#define IR_REPEAT_TIMEOUT 200

boolean LastFrameShowed = true;

DynamicJsonBuffer jsonBuffer;
char input[100];

// GUItool: begin automatically generated code
AudioPlaySdWav Rain;          //xy=102,155
AudioPlaySdWav ThunderLeft;   //xy=112,285
AudioPlaySdWav ThunderRight;  //xy=114,336
AudioPlaySdWav ThunderCenter; //xy=115,401
AudioPlaySdWav *ThunderPLayers[NUMBER_OF_PLAYER] = {&ThunderRight, &ThunderLeft, &ThunderCenter};
AudioMixer4 mixer1;           //xy=366,222
AudioMixer4 mixer2;           //xy=367,300
AudioOutputI2S i2s1;          //xy=569,260
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
    for (int T = 0; T < NUMBER_OF_THUNDERS; T++)
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

    JsonObject &root = jsonBuffer.parseObject(input);
    const char *method = root["method"];
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
  for (uint8_t PlayerCounter = 0; PlayerCounter< NUMBER_OF_PLAYER; PlayerCounter++)
  {
    uint8_t SelectedPlayer = (PlayerCounter+player)%NUMBER_OF_PLAYER;
    if (!ThunderPLayers[SelectedPlayer]->isPlaying())
    {
      playerPointer = ThunderPLayers[SelectedPlayer];
      break;
    }
  }
  //Only trig thunder if we found an available player
  if (playerPointer!=nullptr)
  Thunders[random(0,NUMBER_OF_THUNDERS)]->trig(playerPointer);
}

void initThunders()
{
  H1.addEvent(100, GroupFlash);
  H1.addEvent(200, GroupFlash);
  H1.addEvent(300, GroupFlash);
  H2.addEvent(100, Flash);
  H2.addEvent(200, Flash);
  H2.addEvent(600, GroupFlash);
  H3.addEvent(100, BigFlash);
  H4.addEvent(500, Flash);
  H4.addEvent(1800, GroupFlash);
  H4.addEvent(2000, GroupFlash);
  M1.addEvent(400, BigFlash);
  M1.addEvent(2500, GroupFlash);
  M1.addEvent(3000, BigFlash);
  M2.addEvent(500, Flash);
  M2.addEvent(900, BigFlash);
  M3.addEvent(100, BigFlash);
  M3.addEvent(600, GroupFlash);
  D1.addEvent(1000, Flash);
  D1.addEvent(2500, GroupFlash);
  D1.addEvent(2800, BigFlash);
  D2.addEvent(100, Flash);
  D3.addEvent(2000, Flash);
  D3.addEvent(3000, GroupFlash);
  D4.addEvent(17000, Flash);
  D4.addEvent(1800, Flash);
  D4.addEvent(20000, GroupFlash);
  D4.addEvent(21000, BigFlash);
  D5.addEvent(0, BigFlash);
  VH.addEvent(800, Flash);
  VH.addEvent(1100, MegaFlash);
  VH.addEvent(1500, Flash);
  VH.addEvent(1800, Flash);
  VH.addEvent(2000, Flash);
  VH.addEvent(3500, Flash);
}