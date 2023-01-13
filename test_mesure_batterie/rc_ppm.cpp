#include <Arduino.h>
#include "rc_ppm.h"

//#define PPM_CHANNEL_NB (8)

#define PPM_FRAME_MICRO (22500)

#define PPM_CHANNEL_MAX (2000)
#define PPM_CHANNEL_MIN (1000)

#define PPM_CHANNEL_OFFSET (25)
#define PPM_CHANNEL_VALUE_MIN (0)
#define PPM_CHANNEL_VALUE_MAX (1000)

#define PPM_CHANNEL_ZERO ((PPM_CHANNEL_MAX - PPM_CHANNEL_MIN) / 2)
#define PPM_CHANNEL_SPAN ((PPM_CHANNEL_MAX - PPM_CHANNEL_MIN) / 2)

#define PPM_CHANNELS_MIN (PPM_CHANNEL_NB * PPM_CHANNEL_MIN)
#define PPM_CHANNELS_MAX (PPM_CHANNEL_NB * PPM_CHANNEL_MAX)
#define PPM_FRAME_START_MIN (PPM_FRAME_MICRO - PPM_CHANNELS_MAX)
#define PPM_FRAME_START_MAX (PPM_FRAME_MICRO - PPM_CHANNELS_MIN)

#define PPM_ERROR (5.0 / 100.0)

#define PPM_PULSE_MIN (long)(PPM_CHANNEL_MIN * (1.0 - PPM_ERROR))
#define PPM_PULSE_MAX (long)(PPM_CHANNEL_MAX * (1.0 + PPM_ERROR))

#define PPM_START_PULSE_MIN (long)(PPM_FRAME_START_MIN * (1.0 - PPM_ERROR))
#define PPM_START_PULSE_MAX (long)(PPM_FRAME_START_MAX * (1.0 + PPM_ERROR))

#define PPM_FRAME_LEN_MIN (long)(PPM_FRAME_MICRO * (1.0 - PPM_ERROR))
#define PPM_FRAME_LEN_MAX (long)(PPM_FRAME_MICRO * (1.0 + PPM_ERROR))

//typedef struct {
//    int sync;
//    unsigned long syncStop;
//    int channelMicro[PPM_CHANNEL_NB]; 
//} tPpmStatus;

//static tPpmStatus ppmStatus = {
tPpmStatus ppmStatus = {
    .sync = 0,
    .syncStop = 0,
    //.channelMicro = {0,0,0,0,0,0,0,0}
};

void ppmIsr(void) {
/* This function is called on PPM rising edges */
    unsigned long now,delta;
    static unsigned long ago;
    static int channel;
    volatile tPpmStatus *ppm = &ppmStatus;

    now = micros();
    delta = now - ago;
    // nothing to do on spurious interrupts
    if (delta < 10) return;
    // if micros() timer has rolled over
    if (delta > 2^31) delta = (2^32 - ago) + now;
    //
    // Pulse duration is like a start pulse
    if ((delta >= PPM_START_PULSE_MIN) && (delta <= PPM_START_PULSE_MAX)) {
        ppm->sync = 1;
        channel = 0;
    }
    // Pulse duration is like a channel
    else if ((delta >= PPM_PULSE_MIN) && (delta <= PPM_PULSE_MAX)) {
        if (ppm->sync == 1) {
            delta = delta - PPM_CHANNEL_OFFSET;
            ppm->channelMicro[channel] = delta - PPM_CHANNEL_MIN; 
            if (ppm->channelMicro[channel] < PPM_CHANNEL_VALUE_MIN) {
              ppm->channelMicro[channel] = PPM_CHANNEL_VALUE_MIN;
            }
            if (ppm->channelMicro[channel] > PPM_CHANNEL_VALUE_MAX) {
              ppm->channelMicro[channel] = PPM_CHANNEL_VALUE_MAX;
            }
            channel = (channel + 1) % PPM_CHANNEL_NB;
        }
    }
    // something is wrong
    else {
        // force to wait for the next start of frame
        ppm->sync = 0;
        // can be useful to remember when we lost the sync
        ppm->syncStop = millis(); 
    }
    // prepare next rising edge
    ago = now;

    ppm->lastInterrupt = millis();
}

void rcPpmInit(int inputPin) {
  pinMode(inputPin,INPUT);
  attachInterrupt(digitalPinToInterrupt(inputPin),ppmIsr,RISING);
}

int rcPpmReadChannel(int channelNo,tPpmChannelType type)
{
    tPpmStatus *ppm = &ppmStatus;
    int value;

    if ((channelNo < 0) || (channelNo >= PPM_CHANNEL_NB)) {
        return 0;
    }
    value = ppm->channelMicro[channelNo];
    switch (type) {
        case PPM_JOYSTICK:
            value = value - PPM_CHANNEL_ZERO;
            return (int)((float)value / PPM_CHANNEL_SPAN * 100.0);
            break;
        case PPM_SWITCH_2POS:
            if (value > PPM_CHANNEL_ZERO) return 1;
            else return 0;
            break;
        case PPM_SWITCH_3POS:
            if (value > 2.0 * (2.0 * PPM_CHANNEL_SPAN / 3.0)) return 2;
            else {
                if (value > (2.0 * PPM_CHANNEL_SPAN / 3.0)) return 1;
                else return 0;
            }
            break;
    }
}

int rcPpmIsSync(void) {
    tPpmStatus *ppm = &ppmStatus;
    
    return ppm->sync;
}
