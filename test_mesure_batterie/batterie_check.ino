#include "rc_ppm.h"

int i=0;
void setup()
{
    Serial.begin(115200);
    while(!Serial);

    pinMode(A3,  INPUT);

    rcPpmInit(2);
}

void loop()
{
    
    float batterie = analogRead(A3); //290 pour 15V
    if (i%5==0){
        Serial.println("Batterie tension : " + String(batterie));
    }
    i++;

} 


