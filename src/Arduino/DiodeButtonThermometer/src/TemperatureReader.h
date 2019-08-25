#ifndef TemperatureReader_h
#define TemperatureReader_h

#include <Arduino.h>
#include <OneWire.h>

class TemperatureReader
{
private:
    uint64_t pauseBeforeNextRead;
    std::function<void(byte *, float)> callback;
    OneWire *dsRader;

    int state;
    unsigned long stateSwitchTime;

    // used by OneWire

    byte present;
    byte type_s;
    byte data[12];
    byte addr[8];
    float celsius;

    void handleState0();
    void handleState1();
    void handleState2();
    void handleState3();
protected:
    // empty for now
public:
    TemperatureReader();
    void begin(int, uint64_t);
    void setCallback(std::function<void(byte *, float)>);
    void loop();
};

#endif
