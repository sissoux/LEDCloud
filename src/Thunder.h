#ifndef THUNDER_H
#define THUNDER_H

#include "Arduino.h"
#include "StripLED.h"
#include "FastLED.h"
#include "define.h"
#include "StripCommand.h"
#include "Audio.h"

#define THUNDER_MAX_NUMBER_OF_EVENTS 15

enum FX{
  SingleFlash,
  GroupFlash,
  MegaFlash
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
    AudioPlaySdWav* AudioPlayer_p;

    StripCommand* StripCommander_p;


    Thunder(void);
    Thunder(const char* FileName, StripCommand*);
    int addEvent(uint32_t TimeStamp, FX effect);
    void Update();
    void trig(AudioPlaySdWav*);

    private:
    uint8_t numberOfEvents = 0;
    uint8_t CurrentEvent = 0;
    Event Script[THUNDER_MAX_NUMBER_OF_EVENTS]; 


};


#endif
