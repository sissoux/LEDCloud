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

#define VERSION "0.01"
//Declare an array of pointers to the thunder objects for random selection

Thunder *Thunders[MAX_NUMBER_OF_THUNDERS];
uint8_t NumberOfInitializedThunders = 0;
#define NUMBER_OF_PLAYER 3

elapsedMillis RefreshOutputTimer = 0;
#define OUTPUT_REFRESH_RATE 7
elapsedMillis IRRepeatTimeout = 0;
#define IR_REPEAT_TIMEOUT 200

#define DEBUG_MODE

boolean LastFrameShowed = true;

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
void init_SD();
void init_Player();
ircmd getIRCmd();
void IR_Management();
void taskManager();
void toggleRain();
void startRandomThunder();
void initThunders();
void WriteColorFile(CHSV ColorTable[], int NumberOfColors);
void ParseColorFile(CHSV ColorTable[], int NumberOfColors);

uint8_t Brightness = 0;

void setup()
{
  Serial.begin(115200);
  
#ifdef DEBUG_MODE
  while (!Serial.dtr())
    ;
#endif
Serial.println("Welcome to LEDCLOUD");
  init_SD();
  init_Player();
  Serial.println("Parsing color file.");
  ParseColorFile(IRColorMap, 20);

  FastLED.addLeds<NEOPIXEL, 2>(StripCommander.leds, 0 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, 3>(StripCommander.leds, 1 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, 4>(StripCommander.leds, 2 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, 5>(StripCommander.leds, 3 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  initThunders();

  StripCommander.begin();

  FastLED.show(); //Clear all LEDs
  IR.enableIRIn();
}

void loop()
{
  taskManager();
}

void taskManager()
{
  serialParse();
  IR_Management();

  if (RefreshOutputTimer >= OUTPUT_REFRESH_RATE)
  {
    RefreshOutputTimer = 0;

    for (int T = 0; T < NumberOfInitializedThunders; T++)
    {
      Thunders[T]->Update();
    }
    StripCommander.dynamicStateUpdate();

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
  }
}

void IR_Management()
{
  ircmd cmd = getIRCmd();
  if (IRCmdMap[cmd] != 0)
  {
    StripCommander.setToHSV(IRColorMap[IRCmdMap[cmd]]);
  }
  else
  {
    switch (cmd)
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
}

void init_SD()
{
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN)))
  {
    while (1)
    {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
}

void init_Player()
{
  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(10);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);
}

void serialParse()
{
  if (Serial.available())
  {
    DynamicJsonDocument doc(200);
    deserializeJson(doc, Serial);

    const char *command = doc["command"]; // "setToHSV"

    if (strcmp(command, "fadeToHSV") == 0) //{command:fadeToHSV,H:160,S:255,V:255,Delay:2000}
    {
      int Delay = doc["Delay"]; // 0
      uint8_t H = doc["H"];     // 0
      uint8_t S = doc["S"];     // 1
      uint8_t V = doc["V"];     // 1
      StripCommander.fadeToHSV(H, S, V, Delay);
    }
    else if (strcmp(command, "setToHSV") == 0) //{"command":"setToHSV","H":"255","S":"255","V":"255"}
    {
      uint8_t H = doc["H"]; // 0
      uint8_t S = doc["S"]; // 1
      uint8_t V = doc["V"]; // 1
      StripCommander.setToHSV(H, S, V);
      LastFrameShowed = false;
    }
    else if (strcmp(command, "attributeColor") == 0) //{command:setToHSV,H:255,S:100,V:0}
    {
      uint8_t H = doc["H"]; // 0
      uint8_t S = doc["S"]; // 1
      uint8_t V = doc["V"]; // 1
      int buttonID = doc["ButtonID"];
      StripCommander.setToHSV(H, S, V);
      LastFrameShowed = false;
      IRColorMap[buttonID-1] = CHSV(H, S, V);
    }
    else if (strcmp(command, "callColor") == 0) //{command:setToHSV,H:255,S:100,V:0}
    {
      int buttonID = doc["ButtonID"];
      StripCommander.setToHSV(IRColorMap[buttonID-1]);
      LastFrameShowed = false;
    }
    else if (strcmp(command, "rainbow") == 0) //{command:rainbow}
    {
      StripCommander.rainbow();
      LastFrameShowed = false;
    }
    else if (strcmp(command, "SingleFlash") == 0) //{command:SingleFlash}
    {
      uint8_t FlashCount = random(1, 15);
      for (uint8_t i = 0; i <= FlashCount; i++)
      {
        StripCommander.flash();
      }
    }
    else if (strcmp(command, "BigFlash") == 0) //{command:GroupFlash}
    {
      uint8_t FlashCount = random(20, 35);
      for (uint8_t i = 0; i <= FlashCount; i++)
      {
        StripCommander.flash();
      }
    }
    else if (strcmp(command, "GroupFlash") == 0) //{command:GroupFlash}
    {
      StripCommander.groupFlash();
    }
    else if (strcmp(command, "MegaFlash") == 0) //{command:GroupFlash}
    {
      StripCommander.flashAll();
    }
    else if (strcmp(command, "rain") == 0) //{command:rain}
    {
      toggleRain();
    }
    else if (strcmp(command, "saveColors") == 0) //{command:rain}
    {
      WriteColorFile(IRColorMap, 20);
    }
    else if (strcmp(command, "thunder") == 0) //{command:thunder}
    {
      startRandomThunder();
    }
    else if (strcmp(command, "version") == 0) //{command:thunder}
    {
      Serial.println(VERSION);
    }
    else
    {
      Serial.println("Unsuported command.");
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

void initThunders()
{
  //TODO : read thunder events from a file in SD Card (use JSON)
  //File myFile = SD.open("test.txt", FILE_WRITE);
  //Check if path is valid. If not return without loading black colors;
  if (SD.exists("Thunders.txt"))
  {
    File myFile = SD.open("Thunders.txt");
    if (myFile)
    {
      DynamicJsonDocument JsonDoc(20000);
      DeserializationError error = deserializeJson(JsonDoc, myFile);
      if (!error)
      {
        //If parse is successfull, clear current thunder list and place parsed events.
        NumberOfInitializedThunders = 0;
        JsonArray arr = JsonDoc.as<JsonArray>();
        int NumberOfThundersToAdd = arr.size();
        for (int currentThunder = 0; currentThunder < NumberOfThundersToAdd; currentThunder++)
        {
          JsonObject ThunderToAdd = JsonDoc[currentThunder];
          const char *ObjectType = ThunderToAdd["__type"]; // "Thunder"
          // Check if type is correct, File exists on SD and we do not overflow Thunder list, then create Thunder instance
          if (strcmp(ObjectType, "Thunder") == 1)
          {
            const char *filename = ThunderToAdd["filename"];
            if (SD.exists(filename) && NumberOfInitializedThunders < MAX_NUMBER_OF_THUNDERS)
            {
              int thunder_type = ThunderToAdd["Type"];
              Thunders[NumberOfInitializedThunders] = new Thunder(filename, &StripCommander, (ThunderType)thunder_type);

              //For each event in the script array, add it to the fresh Thunder instance
              JsonArray Script = ThunderToAdd["Script"];
              for (int ScriptIndex = 0; ScriptIndex < ThunderToAdd["NumberOfEvents"]; ScriptIndex++)
              {
                int thunder_fx = Script[ScriptIndex]["fX"];
                Thunders[NumberOfInitializedThunders]->addEvent(Script[ScriptIndex]["timestamp"], (FX)thunder_fx);
              }
              NumberOfInitializedThunders++;
            }
            else
            {
              Serial.print("Wave file ");
              Serial.print(filename);
              Serial.println(" does not exist on SD card or max number of event reached.");
            }
          }
          else
          {
            Serial.println("Content of Object is not Thunder");
          }
        }
      }
      else
      {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
      }
    }
  }
  else
  {
    Serial.println("Thunder definition file does not exist on SD card or max number of event reached.");
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

void ParseColorFile(CHSV ColorTable[], int NumberOfColors)
{
  //Check if path is valid. If not return without loading black colors;
  if (SD.exists("Colors.txt"))
  {
    File myFile = SD.open("Colors.txt");
    if (myFile)
    {
      const size_t capacity = JSON_ARRAY_SIZE(20) + 20 * JSON_OBJECT_SIZE(3) + 140;
      DynamicJsonDocument JsonDoc(capacity);
      DeserializationError error = deserializeJson(JsonDoc, myFile);
      JsonArray arr = JsonDoc.as<JsonArray>();
      if (!error)
      {

        if (arr.size() >= NumberOfColors)
        {
          for (int i = 0; i < NumberOfColors; i++)
          {
            JsonObject root_0 = JsonDoc[i];
            ColorTable[i].h = root_0["H"];
            ColorTable[i].s = root_0["S"];
            ColorTable[i].v = root_0["V"];
          }
        }
        return;
      }
    }
  }
  else Serial.println("\"Colors.txt\" does not exist on SD.");
  for (int i = 0; i < NumberOfColors; i++)
  {
    ColorTable[i] = CHSV(0, 0, 0);
  }
}

void WriteColorFile(CHSV ColorTable[], int NumberOfColors)
{
  const size_t capacity = JSON_ARRAY_SIZE(NumberOfColors) + 20 * JSON_OBJECT_SIZE(3);
  DynamicJsonDocument JsonDoc(capacity);

  for (int i = 0; i < NumberOfColors; i++)
  {
    JsonObject tempJsonObj = JsonDoc.createNestedObject();
    tempJsonObj["H"] = ColorTable[i].h;
    tempJsonObj["S"] = ColorTable[i].s;
    tempJsonObj["V"] = ColorTable[i].v;
  }
  Serial.println("Writing \"Colors.txt\".");
  File file = SD.open("Colors.txt", FILE_WRITE);
  serializeJson(JsonDoc, file);
  serializeJson(JsonDoc, Serial);
  Serial.println();
  file.close();
}
