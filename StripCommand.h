#include "Arduino.h"
#include "StripLED.h"
#include "FastLED.h"
#include "define.h"

#ifndef STRIPCOMMAND_H
#define STRIPCOMMAND_H

class StripCommand
{
  public:
    uint8_t Group[NUM_LEDS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5};
    uint8_t LedPerGroup[NUM_GROUPS] = {11, 11, 11, 11, 11, 1};
    CRGB leds[NUM_LEDS];
    boolean StateChanged = true;
    elapsedMillis tStrip = 0;

    LightMode CurrentMode = Flashing;
    boolean RunningFX = false;

    uint16_t RefreshRate = 0;
    uint16_t FadingDelay = 0;
    uint8_t GradientIndex = 0;
    CHSV StartingColor;
    CHSV TargetColor;
    CRGB GradientBuffer[MAX_TRANSIENT_STEPS];
    boolean FadingDone = true;

    StripLED pixels[NUM_LEDS];

    StripCommand();
    void begin();

    void dynamicStateUpdate();
    
    void flash(uint16_t Id);
    void flash(uint16_t Id, uint16_t Offset);
    void groupFlash(uint8_t gr, uint8_t dir);
    void groupFlash(uint8_t gr);
    void groupFlash();

    void fadeToHSV(uint16_t H, uint16_t S, uint16_t V, uint16_t Delay);
    void setToHSV(uint16_t H, uint16_t S, uint16_t V);
    void fadeToRGB(uint8_t R, uint8_t G, uint8_t B, uint16_t Delay);
    void setToRGB(uint8_t R, uint8_t G, uint8_t B);

    void fadePixelToHSV(uint16_t pixel, float H, float S, float V, uint8_t Delay);
    void setPixelToHSV(uint16_t pixel, float H, float S, float V);
    void fadePixelToRGB(uint16_t pixel, uint16_t R, uint16_t G, uint16_t B, uint8_t Delay);
    void setPixelToRGB(uint16_t pixel, uint16_t R, uint16_t G, uint16_t B);
    void rainbow(void);


    void rainbow(uint8_t wait);
    void colorWipe(uint32_t c, uint8_t wait);
    void rainbowCycle(uint8_t wait);
    void theaterChase(uint32_t c, uint8_t wait);
    void theaterChaseRainbow(uint8_t wait);
    uint32_t Wheel(byte WheelPos);


  private:
};



#endif

