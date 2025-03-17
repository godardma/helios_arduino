#include "rc_ppm.h"

int lastPeriod = 0;
int lastMode   = 0;
int i=0;
int incomingByte = 1; // for incoming serial data
int x;
int left_c=0;
int right_c=0;

const int LED_manual=7;
const int LED_spare=8;

const int voltage_pin=A4;
const int current_pin=A5; 

float volt_divider=11.0;
float amp_divider=37.8788;

unsigned long t0;
unsigned long t0_regul;

int missed_times=0;

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
    pinMode(LED_manual,  OUTPUT);
    pinMode(LED_spare,  OUTPUT);
    pinMode(0,  INPUT);
    pinMode(1,  INPUT);

    pinMode(voltage_pin, INPUT);
    pinMode(current_pin, INPUT);

    rcPpmInit(2);
    t0=millis();
    t0_regul=millis();
    analogReference(EXTERNAL);
}

float convert_tension(float tension_read){
    // return -0.924390068+ 0.026829268*tension_read;
    // return tension_read;
    return (tension_read*3.33/1023.)*21.9/3.9;
}

void loop()
{   
    if ((millis()-t0)>20){
        Serial.flush();
        float batterie_motor = analogRead(0); 
        // Serial.println("Motor " + String(convert_tension(batterie_motor), 4)+" V ");
        float batterie_elec = analogRead(1); 
        // Serial.println("Elec " + String(convert_tension(batterie_elec), 4)+" V ");

        float voltage = analogRead(voltage_pin);        
        float current = analogRead(current_pin);
        voltage=(3.33*voltage/1023.)*volt_divider;
        current=(3.33*current/1023. - 0.3223)*amp_divider;    //0.330 is the offset, ajustement manuel
        // Serial.println("voltage " + String(voltage, 4)+" V");
        // Serial.println("current " + String(current, 4)+" A");
        // Serial.println("left_c = " + String(left_c)+" right_c = " + String(right_c));

        Serial.println("batteries " + String(convert_tension(batterie_motor), 4)+" "+String(convert_tension(batterie_elec), 4)+" "+String(voltage, 4)+" "+String(current, 4));

        t0=millis();
    }
    

    if(!rcPpmIsSync() || millis() - ppmStatus.lastInterrupt > 500) {
        if (missed_times==10000){
            Serial.println("Turning off motors");
            Serial.println(missed_times);
            setBlueRoboticsThrusterPwm(9, 0);
            setBlueRoboticsThrusterPwm(10,0);
            setBlueRoboticsThrusterPwm(11,0);
            setBlueRoboticsThrusterPwm(3, 0);
            left_c=0;
            right_c=0;
	    }
        // Serial.println("No remote control connected. Engines will stay idle.");
        if (missed_times<32)
            missed_times++;
        return;
    }
    else{missed_times=0;}

    int left       = rcPpmReadChannel(0, PPM_JOYSTICK);
    int right      = rcPpmReadChannel(1, PPM_JOYSTICK);
    int modeSwitch = rcPpmReadChannel(4, PPM_SWITCH_2POS);

    if(modeSwitch == 0) {
        if (lastMode!=0){digitalWrite(LED_manual,HIGH);}
        lastMode = modeSwitch;
        left_c=0;
        right_c=0;
        setBlueRoboticsThrusterPwm(9, left);
        setBlueRoboticsThrusterPwm(10, left);
        setBlueRoboticsThrusterPwm(11, right);
        setBlueRoboticsThrusterPwm(3, right);
    }
    else if(modeSwitch == 1) {
        if (lastMode!=1){digitalWrite(LED_manual,LOW);}
        lastMode = modeSwitch;
        String message = Serial.readStringUntil('\n');
        if (message.length()>0){
            int rot, lin, sum;
            sscanf(message.c_str(), "%d %d %d", &rot, &lin, &sum);  // Extraire les deux entiers
            if (sum==lin+rot){
                left_c=lin+rot;
                right_c=lin-rot;
                if (left_c>100){left_c=100;}
                if (left_c<-100){left_c=-100;}
                if (right_c>100){right_c=100;}
                if (right_c<-100){right_c=-100;}
                // Serial.println(String(lin)+" " + String(rot)); 
                setBlueRoboticsThrusterPwm(9, left_c);
                setBlueRoboticsThrusterPwm(10,left_c);
                setBlueRoboticsThrusterPwm(11,right_c);
                setBlueRoboticsThrusterPwm(3, right_c);
            }
        }
        
        // Serial.println("left_c = " + String(left_c));
        // Serial.println("right_c = " + String(right_c));
        // x = Serial.read();
        // x=int(x)-120;
        //Serial.println("commande = " + String(x));
        // if (x>-120 && x<120){
        //     //Serial.println(x);
        //     //Serial.println("Auto mode not implemented.");
        //     //PWM de -100 à 100
        //     if (x<0){
        //    	//if (x>-20){x=-20;}
        //             right_c=60;
        //             left_c=60+x;
        //         }
        //         else {
        //             //if (x<20){x=20;}
        //             left_c=60;
        //             right_c=60-x;
        //         }
        //     /*Serial.println(left_c);
        //     Serial.println(right_c);*/
        //     setBlueRoboticsThrusterPwm(9, left_c);
        //     setBlueRoboticsThrusterPwm(10,left_c);
        //     setBlueRoboticsThrusterPwm(11,right_c);
        //     setBlueRoboticsThrusterPwm(3, right_c);
        //     //setBlueRoboticsThrusterPwm(9, 80);
        //     //setBlueRoboticsThrusterPwm(10,80);
        //     //setBlueRoboticsThrusterPwm(11,0);
        //     //setBlueRoboticsThrusterPwm(3, 0);
        // }
        // else if (x<129 && x>127) {
        //     Serial.println("fin de mission");
        //     setBlueRoboticsThrusterPwm(9, 0);
        //     setBlueRoboticsThrusterPwm(10,0);
        //     setBlueRoboticsThrusterPwm(11,0);
        //     setBlueRoboticsThrusterPwm(3, 0);
        // }
        // else {
        //     /*setBlueRoboticsThrusterPwm(9, left_c);
        //     setBlueRoboticsThrusterPwm(10,left_c);
        //     setBlueRoboticsThrusterPwm(11,right_c);
        //     setBlueRoboticsThrusterPwm(3, right_c);*/
        // }
        
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
