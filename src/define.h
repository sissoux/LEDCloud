#ifndef DEFINE_H
#define DEFINE_H

//#define DEBUG_MODE

#ifdef DEBUG_MODE
#define VERBOSE
#endif

#define NUM_STRIPS 4
#define NUM_LEDS_PER_STRIP 14
#define NUM_LEDS NUM_LEDS_PER_STRIP * NUM_STRIPS
#define NUM_GROUPS 6
#define MAX_TRANSIENT_STEPS 150 // Gives max transition time 5 seconds at 30 update per seconds

#define THUNDER_MAX_NUMBER_OF_EVENTS 10
#define MAX_NUMBER_OF_THUNDERS 30

#endif
