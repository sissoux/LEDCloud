
#include "StripCommand.h"
#include "FastLED.h"

StripCommand::StripCommand()
{
}

void StripCommand::begin()
{
  for (uint8_t i = 0; i < NUM_LEDS; i++)
  {
    pixels[i].init(i, Group[i]);
  }
}

void StripCommand::dynamicStateUpdate()
{
  RunningFX = false;
  switch (CurrentMode)
  {
  case Flashing:
  {
    for (uint8_t i = 0; i < NUM_LEDS; i++)
    {
      leds[i] = pixels[i].update();
      RunningFX |= pixels[i].Active;
    }
  }
  break;

  case Fading:
    if (!FadingDone)
    {
      RunningFX = true;
      if (tStrip >= RefreshRate)
      {
        while (tStrip >= RefreshRate)
        {
          tStrip -= RefreshRate;
        }
        if (GradientIndex < 150)
        {
          GradientIndex++;
          fill_solid(leds, NUM_LEDS, GradientBuffer[GradientIndex - 1]);
          return;
        }
        else
        {
          fill_solid(leds, NUM_LEDS, TargetColor);
          RunningFX = false;
          FadingDone = true;
        }
      }
    }
    break;

  case Rainbow:
    if (RainbowTimer >= RainbowRate)
    {
      RainbowRate = 0;
      CurrentRainbowStartingHue +=1;
      this->rainbow();
    }
    break;

  default:
    break;
  }
}

void StripCommand::flash()
{
  if (CurrentMode != Flashing && RunningFX)
    return;
  CurrentMode = Flashing;
  uint16_t Id = random(0, NUM_LEDS);
  pixels[Id].Mode = Flashing;
  pixels[Id].Offset = random(20, 300);
  pixels[Id].Trigger = 1;
}

void StripCommand::flash(uint16_t Id)
{
  if (CurrentMode != Flashing && RunningFX)
    return;
  CurrentMode = Flashing;
  pixels[Id].Mode = Flashing;
  pixels[Id].Offset = random(20, 300);
  pixels[Id].Trigger = 1;
}

void StripCommand::flash(uint16_t Id, uint16_t Offset)
{
  if (CurrentMode != Flashing && RunningFX)
    return;
  CurrentMode = Flashing;
  pixels[Id].Mode = Flashing;
  pixels[Id].Offset = Offset;
  pixels[Id].Trigger = 1;
}

void StripCommand::groupFlash(uint8_t gr, uint8_t dir)
{
  if (CurrentMode != Flashing && RunningFX)
    return;

  CurrentMode = Flashing;
  uint8_t spd = random(2, 25);
  uint16_t Multiplier = 0;
  for (uint8_t i = 0; i < NUM_LEDS; i++)
  {
    if (Group[i] == gr)
    {
      if (dir == 0)
      {
        flash(i, spd * Multiplier);
      }
      else
      {
        flash(i, spd * LedPerGroup[gr] - spd * Multiplier);
      }
      Multiplier++;
    }
  }
}

void StripCommand::groupFlash(uint8_t gr)
{
  uint8_t dir = random(0, 1);
  groupFlash(gr, dir);
}

void StripCommand::groupFlash()
{
  uint8_t gr = random(0, NUM_GROUPS - 1);
  uint8_t dir = random(0, 2);
  groupFlash(gr, dir);
}

void StripCommand::flashAll()
{
  if (CurrentMode != Flashing && RunningFX)
    return;
  CurrentMode = Flashing;
  for (uint16_t Id = 0; Id < NUM_LEDS; Id++)
  {
    pixels[Id].Mode = Flashing;
    pixels[Id].Offset = random(20, 300);
    pixels[Id].Trigger = 1;
  }
}

void StripCommand::fadeToHSV(uint8_t H, uint8_t S, uint8_t V, uint16_t Delay)
{
  fadeToHSV(CHSV(H, S, V), Delay);
}

void StripCommand::fadeToHSV(CHSV target, uint16_t Delay)
{
  if ((CurrentMode != Fading && RunningFX) || (CurrentMode == Fading && !FadingDone))
    return;

  CurrentMode = Fading;
  StartingColor = rgb2hsv_approximate(leds[0]);
  TargetColor = target;
  FadingDelay = Delay;
  FadingDone = false;


  RefreshRate = FadingDelay / MAX_TRANSIENT_STEPS;
  fill_gradient(GradientBuffer, 0, StartingColor, MAX_TRANSIENT_STEPS - 1, TargetColor);
  GradientIndex = 0;
  tStrip = 0;
  
}

void StripCommand::setToHSV(CHSV target)
{
  fill_solid(leds, NUM_LEDS, target);
  StateChanged = true;
}

void StripCommand::setToHSV(uint8_t H, uint8_t S, uint8_t V)
{
  setToHSV(CHSV(H, S, V));
}

void StripCommand::fadeToRGB(uint8_t R, uint8_t G, uint8_t B, uint16_t Delay)
{
  CHSV myColor = rgb2hsv_approximate(CRGB(R, G, B));
  fadeToHSV(myColor.h, myColor.s, myColor.v, Delay);
}

void StripCommand::setToRGB(uint8_t R, uint8_t G, uint8_t B)
{
  fill_solid(leds, NUM_LEDS, CRGB(R, G, B));
  StateChanged = true;
}

void StripCommand::rainbow()
{
  if (CurrentMode != Rainbow && RunningFX)
  {
    CurrentMode = Rainbow;
  }
  fill_rainbow(leds, 56, CurrentRainbowStartingHue, 5);
  StateChanged = true;
}

void StripCommand::rainbow(uint32_t rate)
{
  if (rate!=0) RainbowRate = rate;
  rainbow();
}
