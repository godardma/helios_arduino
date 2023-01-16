#include "rc_ppm.h"

int lastPeriod = 0;
int lastMode   = 0;

void setBlueRoboticsThrusterPwm(int pin,int percentCmd);

void setup()
{
    Serial.begin(115200);
    while(!Serial);

    pinMode(9,  OUTPUT);
    pinMode(10, OUTPUT);
    pinMode(11, OUTPUT);
    pinMode(3,  OUTPUT);

    rcPpmInit(2);
}

void loop()
{
    if(!rcPpmIsSync() || millis() - ppmStatus.lastInterrupt > 500) {
        setBlueRoboticsThrusterPwm(9, 0);
        setBlueRoboticsThrusterPwm(10,0);
        setBlueRoboticsThrusterPwm(11,0);
        setBlueRoboticsThrusterPwm(3, 0);

        Serial.println("No remote control connected. Engines will stay idle.");

        return;
    }

    int left       = rcPpmReadChannel(0, PPM_JOYSTICK);
    int right      = rcPpmReadChannel(1, PPM_JOYSTICK);
    int modeSwitch = rcPpmReadChannel(4, PPM_SWITCH_2POS);

    if(modeSwitch == 0) {
        lastMode = modeSwitch;
        setBlueRoboticsThrusterPwm(9, left);
        setBlueRoboticsThrusterPwm(10, left);
        setBlueRoboticsThrusterPwm(11, right);
        setBlueRoboticsThrusterPwm(3, right);
    
        Serial.write("manu "); Serial.print(left);
        Serial.write(' ');    Serial.print(right);
        Serial.println("");
    }
    else if(modeSwitch == 1) {
        lastMode = modeSwitch;
        Serial.println("Auto mode not implemented.");
        setBlueRoboticsThrusterPwm(9, 0);
        setBlueRoboticsThrusterPwm(10,0);
        setBlueRoboticsThrusterPwm(11,0);
        setBlueRoboticsThrusterPwm(3, 0);
    }
    else {
        Serial.println("FATAL : Invalid control mode.");
        setBlueRoboticsThrusterPwm(9, 0);
        setBlueRoboticsThrusterPwm(10,0);
        setBlueRoboticsThrusterPwm(11,0);
        setBlueRoboticsThrusterPwm(3, 0);
    }
} 

#define ARDUINO_PWM_ZERO (186)
#define ARDUINO_THRUSTER_UP_START (189)
#define ARDUINO_THRUSTER_DOWN_START (184)
#define ARDUINO_THRUSTER_MAX (236)    // +100%
#define ARDUINO_THRUSTER_MIN (136)    // -100%

#define THRUSTER_DEAD_ZONE (3)

void setBlueRoboticsThrusterPwm(int pin,int percentCmd) {
// pin: number of the pin carrying the pwm signal
// -100 <= percentCmd <= 100
// Blue Robotics thruster PWM: 
// period: 2000µs  1100µs <= high level <= 1900µs
// 1480 <= high level <= 1520: blank zone (no thruster rotation)
// Arduino PWM cmd (default PWM period is 2049µs (488Hz)):
// 0: 0µs high level
// 127: 1016µs high level
// 136(.90): 1100µs high level
// 184(.19): 1480µs high level
// 186(.68): 1500µs high level
// 189(.17): 1520µs high level
// 236(.46): 1900µs high level
// 255: 2049µs high level
    int cmd;

    percentCmd /= 2;
    if (percentCmd > THRUSTER_DEAD_ZONE) {
        cmd = ARDUINO_THRUSTER_UP_START + percentCmd;
        if (cmd > ARDUINO_THRUSTER_MAX) cmd = ARDUINO_THRUSTER_MAX;
    }
    else if (percentCmd < -THRUSTER_DEAD_ZONE) {
        cmd = ARDUINO_THRUSTER_DOWN_START + percentCmd;
        if (cmd < ARDUINO_THRUSTER_MIN) cmd = ARDUINO_THRUSTER_MIN;
    }
    else cmd = ARDUINO_PWM_ZERO;
    //
    analogWrite(pin,cmd);
}
