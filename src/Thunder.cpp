#include "Thunder.h"

Thunder::Thunder(const char *FileName, StripCommand *_stripCommander, ThunderType _type)
{
    this->filename = FileName;
    this->StripCommander_p = _stripCommander;
    this->Type = _type;
}

void Thunder::Update()
{ //If this thunder is active, and has been properly initialized, we check the timestand versus script events and apply correspondig effects
    if (this->numberOfEvents == 0)
        return;

    if (this->Active)
    {
        if (this->CurrentEvent < this->numberOfEvents)
        {
            if (this->t >= this->Script[this->CurrentEvent].timestamp)
            {
                switch (this->Script[this->CurrentEvent].effect)
                {
                case SingleFlash:
                {
                    uint8_t FlashCount = random(1, 15);
                    for (uint8_t i = 0; i <= FlashCount; i++)
                    {
                        this->StripCommander_p->flash();
                    }
                }
                break;
                case BigFlash:
                {
                    uint8_t FlashCount = random(20, 35);
                    for (uint8_t i = 0; i <= FlashCount; i++)
                    {
                        this->StripCommander_p->flash();
                    }
                }
                break;

                case GroupFlash:
                    this->StripCommander_p->groupFlash();
                    break;

                case MegaFlash:
                    this->StripCommander_p->flashAll();
                    break;

                default:
                    break;
                }
                this->CurrentEvent++;
            }
        }
        else
        {
            this->Active = false;
        }
    }
}

void Thunder::trig(AudioPlaySdWav *Player_p)
{ //If audioChannel is free, then we start the thunder wave file and launch associated FX script by setting Active
    if (Player_p->isPlaying())
        return;
    this->AudioPlayer_p = Player_p;
    this->Active = true;
    this->CurrentEvent = 0;
    this->AudioPlayer_p->play(this->filename);
    this->t = 0;
}

//Add one event to the event list, sort them so the timestamp is increasing.
int Thunder::addEvent(uint32_t TimeStamp, FX effect)
{
    if (this->numberOfEvents < THUNDER_MAX_NUMBER_OF_EVENTS)
    {
        if (numberOfEvents != 0)
        {
            //Go through the elements to keep increasing order
            for (int index = 0; index <= numberOfEvents; index++)
            {
                if (index != numberOfEvents)
                {
                    if (TimeStamp < this->Script[numberOfEvents - 1].timestamp)
                    {
                        for (int shiftIndex = numberOfEvents; shiftIndex > index; shiftIndex--)
                        {
                            this->Script[shiftIndex].timestamp = this->Script[shiftIndex - 1].timestamp;
                            this->Script[shiftIndex].effect = this->Script[shiftIndex - 1].effect;
                        }
                        this->Script[index] = {TimeStamp, effect};
                        break;
                    }
                }
                else
                { //Timestamp is higher than all elements, add it at the end
                    this->Script[numberOfEvents] = {TimeStamp, effect};
                }
            }
        }
        else
        { //Timestamp is higher than all elements, add it at the end
            this->Script[numberOfEvents] = {TimeStamp, effect};
        }
        numberOfEvents++;
        return 1;
    }
    else
        return -1; //Return error, could not add event as list is already full
}

//Add a list of events to the event list, sort them so the timestamp is increasing.
int Thunder::addEvent(Event eventlist[], uint8_t NumberOfEventToAdd)
{
    if ((this->numberOfEvents + NumberOfEventToAdd) < THUNDER_MAX_NUMBER_OF_EVENTS && sizeof(eventlist) == sizeof(Event)*NumberOfEventToAdd)
    {
        for (int event = 0; event < NumberOfEventToAdd; event++)
        {
            this->addEvent(eventlist[event].timestamp, eventlist[event].effect);
        }
    }
    else
        return -1; //Return error, could not add event as list is already full
}

void Thunder::purgeEventList()
{
    this->numberOfEvents = 0; //No need to delete the events, just pretend the list is empty
}