#include "LED.h"
#include "Arduino.h"


LED::LED()
{

}

LED::LED(unsigned int ID, unsigned int Tfall)
{
  this->ID = ID;
  this->Tf = Tfall;
}



void LED::update(pixelState State)
{
  switch (State)
  {
  case FLASH:
    this->flash();
    break;

  case CHAINEDFLASH:

    break;

  case STATIC:

    break;

  case COLORFADING:

    break;

  case RANDOMCOLOR:

    break;

  case BLACKOUT:
    this->R = 0;
    this->G = 0;
    this->B = 0;
    break;
  }
}



void LED::flash()
{
  if (this->Trig)                                          //if this LED is trigerred clear trigger flag, set active flag mark begining of flash
  {
    this->Trig = 0;
    this->Active = 1;
    this->StartFlag = millis();
  }

  if (this->Active)
  {
    int tick = millis()-this->StartFlag;
    if (tick < this->Tr)                                  // We are in rising phase
    {
      this->R = (this->FlashIntensity*(tick)/Tr);
      this->G = (this->FlashIntensity*(tick)/Tr);
      this->B = (this->FlashIntensity*(tick)/Tr);
    }
    else if (tick < (this->Tr + this->Ts))                 // We are in flat phase
    {
      this->R = this->FlashIntensity;
      this->G = this->FlashIntensity;
      this->B = this->FlashIntensity;
    }
    else if (tick < (this->Tr + this->Ts + this->Tf))       // We are in falling phase
    {
      this->R = this->FlashIntensity-(this->FlashIntensity*(tick-Tr-Ts)/Tf);
      this->G = this->FlashIntensity-(this->FlashIntensity*(tick-Tr-Ts)/Tf);
      this->B = this->FlashIntensity-(this->FlashIntensity*(tick-Tr-Ts)/Tf);
    }
    else
    {
      this->Active = 0;
    }
  }
  else
  {
    this->R = 0;
    this->G = 0;
    this->B = 0;
  }
}

void LED::chainedFlash()
{

}
void LED::color()
{

}
void LED::colorFading()
{

}
void LED::randomColor()
{

}
void LED::blackout()
{

}




