#include "Arduino.h"
#include "esp_task_wdt.h"
#include <phaseAngleControl.cpp>

const int triacPin = 2;
const int potPin = 27;
const int brushPin = 13;
const int hallPin = 14;

const float pGain = 10000;
const float iGain = .05;
const float piGain = 1;

const int pulseDuration = 500;
const int minimumDelay = 150;

int triggerDelay = minimumDelay;
float triggerDelayFloat = triggerDelay;
float targetfrequency = 0;

float currentIntegral = 0;
float currentProportional = 0;

unsigned long prevTime = 0;
unsigned long prevDuration = 0;
unsigned long prevRising = 0;
unsigned long prevFalling = 0;

void handleSetFrequency(String message)
{
    float newTargetFrequency = message.toFloat();

    targetfrequency = newTargetFrequency;
    /*currentIntegral = 0;
    currentProportional = 0;*/

    Serial.println("sf" + String(targetfrequency));
}

void handleMessage(String text)
{
    char func = text[0];
    String message = text.substring(1);

    switch (func)
    {
    case 'p':
        Serial.println("sPing" + message);
        break;
    case 's':
        handleSetFrequency(message);
        break;
    default:
        Serial.println("eUnknownFuction");
    }

    return;
}

void handleHalfRevolution(unsigned long time)
{
    if (prevTime == 0)
    {
        prevTime = time;
        return;
    }

    int duration = time - prevTime;

    if (duration < 23000)
    {
        return;
    }

    prevDuration = duration;
    prevTime = time;
}

unsigned long delta = 0;

void handleHallFalling()
{
    unsigned long time = micros();
    prevFalling = time;

    delta = time - prevRising;

    if (time - prevRising > 15000)
    {
        handleHalfRevolution(time);
    }
}

void handleHallRising()
{
    unsigned long time = micros();
    prevRising = time;

    handleHalfRevolution(time);
}

void handleHallChange()
{
    unsigned long time = micros();

    int state = digitalRead(brushPin);
    if (state == HIGH)
    {
        prevRising = time;

        handleHalfRevolution(time);
        return;
    }

    prevFalling = time;

    delta = time - prevRising;

    if (time - prevRising > 3500)
    {
        handleHalfRevolution(time);
    }
    // handleHalfRevolution(time);
}
PhaseAngleController *pac = new PhaseAngleController(18, 2, 0, 150, 500);
void setup()
{
    pinMode(triacPin, OUTPUT);
    pinMode(potPin, INPUT);
    pinMode(brushPin, INPUT_PULLDOWN);
    pinMode(hallPin, INPUT_PULLUP);

    digitalWrite(triacPin, HIGH);
    Serial.begin(115200);
    // touchAttachInterrupt(hallPin, handleHallRising, )

    //attachInterrupt(digitalPinToInterrupt(hallPin), handleHallRising, FALLING);
    // attachInterrupt(digitalPinToInterrupt(brushPin), handleHallRising, RISING);
    targetfrequency = 5;

    // esp_task_wdt_init(60000, false);
}

float prevHz = 0;
float prevcor = 0;
unsigned long prevDelta = 0;
unsigned long prevMicros = 0;
bool failed = false;
bool prevIr = false;
bool ir = false;

void loop()
{
    if (Serial.available() > 0)
    {
        String incoming = Serial.readStringUntil('\n');

        handleMessage(incoming);
    }

    int currIr = analogRead(hallPin);

    Serial.println(currIr);

    if (currIr > 2048) {
        ir = false;
    }

    if (currIr < 1500) {
        ir = true;
    }

    if (ir && !prevIr) {
        handleHallRising();
    }

    prevIr = ir;

    /* TODO: pi controller logic */
    // Serial.println(micros()- prevMicros);
    pac->loop();

    if (millis() < 2000) {
        return;
    }

    pac->triggerDelay = (analogRead(27) / 4095.0) * 9500;
    //return;
    if (failed)
    {
        return;
    }

    if (triggerDelayFloat < 0)
    {
        triggerDelayFloat = 0;
    }
    if (triggerDelayFloat > 10000 - minimumDelay - pulseDuration - 60)
    {
        triggerDelayFloat = 10000 - minimumDelay - pulseDuration - 60;
    }
    int temp = static_cast<int>(triggerDelayFloat);

    //pac->triggerDelay = temp;
    // Serial.println(prevTime);

    float hz = 1 / (prevDuration / 1000000.0);

    if (abs(prevHz - hz) > 10 || hz > 19)
    {
        return;
    }
    if (abs(prevHz - hz) > .01)
    {
        Serial.print(hz);
        Serial.println("Hz");
    }

    prevHz = hz;
    prevDelta = delta;

    unsigned long timeDelta = micros() - prevMicros;

    float error = targetfrequency - hz;

    if (isinf(error))
    {
        return;
    }
    currentIntegral += error * timeDelta * 0.000001;

    float correction = -piGain * (currentIntegral * iGain + error * pGain);

    if (abs(correction - prevcor) > 10)
    {
        prevcor = correction;

        // Serial.println(correction);
    }

    triggerDelayFloat += correction;

    // Serial.println(pac->triggerDelay);

    prevMicros = micros();
}
