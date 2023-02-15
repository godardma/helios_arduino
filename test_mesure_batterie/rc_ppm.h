#ifndef __RC_PPM__
#define __RC_PPM__

#define PPM_CHANNEL_NB (8)

typedef struct {
    int sync;
    unsigned long syncStop;
    unsigned long lastInterrupt; // for check
    int channelMicro[PPM_CHANNEL_NB]; 
} tPpmStatus;

typedef enum {
    PPM_JOYSTICK,
    PPM_SWITCH_2POS,   // 2 positions switch
    PPM_SWITCH_3POS    // 3 positions switch
} tPpmChannelType;

void rcPpmInit(int inputPin);
int rcPpmReadChannel(int channel,tPpmChannelType type);
int rcPpmIsSync(void);

extern tPpmStatus ppmStatus;

#endif
