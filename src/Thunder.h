#ifndef THUNDER_H
#define THUNDER_H

#include "Arduino.h"
#include "StripLED.h"
#include "FastLED.h"
#include "define.h"
#include "StripCommand.h"
#include "Audio.h"



enum FX{
  SingleFlash = 1,
  BigFlash = 2,
  GroupFlash = 3,
  MegaFlash = 4
};

enum ThunderType{
  Distant = 1, 
  Medium = 2, 
  Heavy = 3, 
  VeryHeavy = 4
};

struct Event{
  uint32_t timestamp = 0;
  enum FX effect = SingleFlash;
};

class Thunder
{
    public: 
    
    boolean Trigger = false;
    boolean Active = false;
    boolean LastStep = false;

    elapsedMillis t;
    const char* filename;
    enum ThunderType Type;
    AudioPlaySdWav* AudioPlayer_p;

    StripCommand* StripCommander_p;


    Thunder(void);
    Thunder(const char* FileName, StripCommand*, ThunderType);
    int addEvent(uint32_t TimeStamp, FX effect);
    int addEvent(Event eventlist[], uint8_t NumberOfEvents);
    void purgeEventList();
    void Update();
    void trig(AudioPlaySdWav*);

    private:
    uint8_t numberOfEvents = 0;
    uint8_t CurrentEvent = 0;
    Event Script[THUNDER_MAX_NUMBER_OF_EVENTS]; 


};


#endif
