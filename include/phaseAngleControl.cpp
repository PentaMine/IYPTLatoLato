#include <Arduino.h>

class PhaseAngleController
{
private:
    uint32_t nextPulseStart;
    uint32_t nextPulseEnd;
    bool prevFeedState;

public:
    uint32_t triggerDelay = 0;
    uint32_t minimumDelay = 0;
    uint32_t pulseDuraton = 0;
    int feedPin;
    int triacPin;
    PhaseAngleController(int feedPin, int triacPin, uint32_t initialDelay, uint32_t minimumDelay, uint32_t pulseDuration)
    {
        this->feedPin = feedPin;
        this->triacPin = triacPin;
        this->triggerDelay = initialDelay;
        this->minimumDelay = minimumDelay;
        this->pulseDuraton = pulseDuration;
    }

    void loop()
    {
        uint32_t time = micros();

        if (time > this->nextPulseStart)
        {
            digitalWrite(triacPin, LOW);
            this->nextPulseStart += 40000;
        }

        if (time > this->nextPulseEnd)
        {
            digitalWrite(triacPin, HIGH);
            this->nextPulseEnd += 40000;
        }

        bool feedState = digitalRead(feedPin);

        if (feedState != prevFeedState)
        {
            if (this->triggerDelay == 0)
            {
                this->nextPulseStart = time - 400000;
                this->nextPulseEnd = time + 400000;
                return;
            }
            else if (this->triggerDelay >= 10000 - this->minimumDelay - this->pulseDuraton - 60)
            {
                this->nextPulseStart = time + 400000;
                this->nextPulseEnd = time - 400000;
                return;
            }

            this->nextPulseStart = time + this->minimumDelay + this->triggerDelay - 60;
            this->nextPulseEnd = this->nextPulseStart + this->pulseDuraton;
        }
        prevFeedState = feedState;
    }
};