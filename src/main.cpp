#include "Arduino.h"

const int ledPin = 2;
int triggerDelay = 0;
const int pulseDuration = 500;
const int minimumDelay = 150;

void handleInputChangeInterrupt()
{
    delayMicroseconds(triggerDelay);
    digitalWrite(ledPin, LOW);
    delayMicroseconds(pulseDuration);
    digitalWrite(ledPin, HIGH);
}

void setup()
{
    // setup pin 5 as a digital output pin
    pinMode(ledPin, OUTPUT);
    attachInterrupt(18, handleInputChangeInterrupt, CHANGE);
    digitalWrite(ledPin, HIGH);
    Serial.begin(115200);
}

void loop()
{
    triggerDelay = minimumDelay + (analogRead(27) / 4095.0) * (10000 - minimumDelay - pulseDuration - 40);
    Serial.println(triggerDelay);
}
