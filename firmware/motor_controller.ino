#include "rc_ppm.h"

int lastPeriod = 0;
int lastMode   = 0;
int i=0;
int incomingByte = 1; // for incoming serial data
int x;
int left_c;
int right_c;

void setBlueRoboticsThrusterPwm(int pin,int percentCmd);

void setup()
{
    Serial.begin(115200);
    Serial.setTimeout(1);
    while(!Serial);

    pinMode(9,  OUTPUT);
    pinMode(10, OUTPUT);
    pinMode(11, OUTPUT);
    pinMode(3,  OUTPUT);
    pinMode(0,  INPUT);

    rcPpmInit(2);
}

float convert_tension(float tension_read){
    return 0.104510437284317 + 0.020225595443943*tension_read;
}

void loop()
{
    float batterie = analogRead(0); //495 pour 10.1V, 582 pour 11.9, 677 pour 13.8, 737 pour 15
    //Serial.println("Batterie tension = " + String(convert_tension(batterie))+"V");

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
    
        Serial.write("manu "); Serial.println(left);
        Serial.write(' ');    Serial.println(right);
        Serial.println("");
    }
    else if(modeSwitch == 1) {
        lastMode = modeSwitch;
        x = Serial.read();
        x=int(x)-120;
        //Serial.println("commande = " + String(x));
        if (x>-120 && x<120){
            //Serial.println(x);
            //Serial.println("Auto mode not implemented.");
            //PWM de -100 à 100
            if (x<0){
            	//if (x>-20){x=-20;}
                right_c=80;
                left_c=80+x;
            }
            else {
            	//if (x<20){x=20;}
                left_c=80;
                right_c=80-x;
            }
            /*Serial.println(left_c);
            Serial.println(right_c);*/
            setBlueRoboticsThrusterPwm(9, left_c);
            setBlueRoboticsThrusterPwm(10,left_c);
            setBlueRoboticsThrusterPwm(11,right_c);
            setBlueRoboticsThrusterPwm(3, right_c);
            //setBlueRoboticsThrusterPwm(9, 80);
            //setBlueRoboticsThrusterPwm(10,80);
            //setBlueRoboticsThrusterPwm(11,0);
            //setBlueRoboticsThrusterPwm(3, 0);
        }
        else if (x<129 && x>127) {
            Serial.println("fin de mission");
            setBlueRoboticsThrusterPwm(9, 0);
            setBlueRoboticsThrusterPwm(10,0);
            setBlueRoboticsThrusterPwm(11,0);
            setBlueRoboticsThrusterPwm(3, 0);
        }
        else {
            /*setBlueRoboticsThrusterPwm(9, left_c);
            setBlueRoboticsThrusterPwm(10,left_c);
            setBlueRoboticsThrusterPwm(11,right_c);
            setBlueRoboticsThrusterPwm(3, right_c);*/
        }
        
    }
    else {
        Serial.println("FATAL : Invalid control mode.");
        setBlueRoboticsThrusterPwm(9, 0);
        setBlueRoboticsThrusterPwm(10,0);
        setBlueRoboticsThrusterPwm(11,0);
        setBlueRoboticsThrusterPwm(3, 0);
    }
    i++;
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
