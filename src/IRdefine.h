#ifndef IRDEFINE_H
#define IRDEFINE_H

#include "Arduino.h"

#define IR_PIN 8
#define NB_IR_CMD 44

uint16_t IRcmdCommands[NB_IR_CMD] = {0x3AC5, 0x1AE5, 0x2AD5, 0x0AF5, 0x38C7, 0x18E7, 0x28D7, 0x08F7, 0x30CF, 0x10EF, 0x20DF, 0xBA45, 0x9A65, 0xAA55, 0x8A75, 0xB847, 0x9867, 0xA857, 0x8877, 0xB04F, 0x906F, 0xA05F, 0x827D, 0xA25D, 0x926D, 0xB24D, 0x7887, 0x58A7, 0x6897, 0x48B7, 0x708F, 0x50AF, 0x609F, 0x02FD, 0x22DD, 0x12ED, 0x32CD, 0xF807, 0xD827, 0xE817, 0xC837, 0xF00F, 0xD02F, 0xE01F};

typedef enum {
  Ip,
  R,
  Ca,
  Ce,
  Ci,
  Cm,
  Rp,
  Rm,
  DIYa,
  DIYd,
  Jumpa,
  Im,
  G,
  Cb,
  Cf,
  Cj,
  Cn,
  Gp,
  Gm,
  DIYb,
  DIYe,
  Jumpb,
  Play,
  B,
  Cc,
  Cg,
  Ck,
  Co,
  Bp,
  Bm,
  DIYc,
  DIYf,
  Fadea,
  Pwr,
  W,
  Cd,
  Ch,
  Cl,
  Cp,
  Spdp,
  Spdm,
  Auto,
  Flash,
  Fadeb,
  NO_CMD,
  RPT
} ircmd;

ircmd MyCMD = NO_CMD;
IRrecv IR(IR_PIN);
decode_results RawIRCmd;
ircmd LastIRCmd = NO_CMD;


#endif

