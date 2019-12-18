#include "StripLED.h"

StripLED::StripLED()
{
}

void StripLED::init(uint16_t ID, uint8_t Group)
{
  this->ID = ID;
  this->Group = Group;
}

CRGB StripLED::update()
{
  if (Trigger && !Active)
  {
    Active = true;
    Trigger = false;
    if (this->Mode == Flashing)
    {
      TRise = random(5, 25);
      THigh = random(5, 25);
      TFall = random(50, 200);
      SumTRise = TRise + Offset;
      SumTHigh = SumTRise + THigh;
      SumTFall = SumTHigh + TFall;
      UpStep = 100 * MaxBrightness / TRise;
      DownStep = 100 * MaxBrightness / TFall;
    }
    t = 0;
  }
  if (Active)
  {
    uint16_t T = t;
    if (this->Mode == Flashing)
    {
      if (T <= Offset)
      {
        Brightness = 0;
      }
      else if (T <= SumTRise)
      {
        Brightness = (UpStep * T) / 100;
      }
      else if (T <= SumTHigh)
      {
        Brightness = MaxBrightness;
      }
      else if (T <= SumTFall)
      {
        Brightness = (MaxBrightness - DownStep * (T - SumTHigh) / 100);
      }
      else
      {
        Active = false;
        LastStep = true;
        Brightness = 0;
      }
      return CRGB(CHSV(0, 0, Brightness));
    }
  }
  return CRGB::Black;
}
