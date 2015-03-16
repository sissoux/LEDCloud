#include "Arduino.h"

#define  INT 0
#define  TR 1
#define  TS 2
#define  TF 3
#define  ACTIVE 4
#define  TRIG 5
  
  typedef enum{           //Goal of this enum is to define different states in which the pixel can be. Normally state should be the same for all Pixels.
    FLASH,                //This state is used to generate random duration flash on random LED
    CHAINEDFLASH,         //This state is used to generate a random duration flash serie on all the strip length specifying start and direction
    STATIC,               //This state is used to place LED in a static color
    COLORFADING,          //This state generate a fading from one color to another
    RANDOMCOLOR,          //This state generate random colors at random durations and fade between them
    BLACKOUT              //This turn off the LED
  }pixelState;
  

class LED
{
  public:
  LED();
  LED(unsigned int ID, unsigned int Tfall);
  
  pixelState PreviousState = BLACKOUT;
  
  void flash();
  void chainedFlash();
  void color();
  void colorFading();
  void randomColor();
  void blackout();
  void update(pixelState);
  
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
