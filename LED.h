#include "Arduino.h"

#define  INT 0
#define  TR 1
#define  TS 2
#define  TF 3
#define  ACTIVE 4
#define  TRIG 5


class LED
{
  
  public:
  LED();
  LED(unsigned int, unsigned int);
  
  void flash();
  
  unsigned int Tr = 1;
  unsigned int Ts = 0;
  unsigned int Tf = 300;
  byte FlashIntensity = 255;
  boolean Active = 0;
  boolean Trig = 0;
  unsigned int R,G,B;
  unsigned int ID;
  
  private:
  unsigned long StartFlag = 0;
  
  
};
