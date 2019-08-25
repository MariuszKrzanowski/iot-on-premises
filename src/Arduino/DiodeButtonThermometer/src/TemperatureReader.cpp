// Based on:
// OneWire DS18S20, DS18B20, DS1822 Temperature Example
//
// http://www.pjrc.com/teensy/td_libs_OneWire.html
//
// The DallasTemperature library can do all this work for you!
// https://github.com/milesburton/Arduino-Temperature-Control-Library
//
// Adopted by Mariusz Krzanowski to use with MQTT Broker

#include <Arduino.h>
#include <OneWire.h>
#include "TemperatureReader.h"

#define TEMPERATURE_READER_WRITE_TO_SERIAL_NO

TemperatureReader::TemperatureReader()
{
    this->dsRader = NULL;
    this->callback = NULL;

    this->state = 0;
    this->stateSwitchTime = millis();

    this->present = 0;
    this->type_s = 0;
    this->celsius = 0.0;
}

void TemperatureReader::setCallback(std::function<void(byte *, float)> callback)
{
    this->callback = callback;
}

void TemperatureReader::handleState0()
{
    if (!this->dsRader->search(addr))
    {
#ifdef TEMPERATURE_READER_WRITE_TO_SERIAL
        Serial.println("No more addresses.");
        Serial.println();
#endif
        this->dsRader->reset_search();
        state = 1;
        stateSwitchTime = millis();
        return;
    }

    if (OneWire::crc8(addr, 7) != addr[7])
    {
#ifdef TEMPERATURE_READER_WRITE_TO_SERIAL
        Serial.println("CRC is not valid!");
#endif
        return;
    }

    // the first ROM byte indicates which chip
    switch (addr[0])
    {
    case 0x10:
#ifdef TEMPERATURE_READER_WRITE_TO_SERIAL
        Serial.println("  Chip = DS18S20"); // or old DS1820
#endif
        type_s = 1;
        break;
    case 0x28:
#ifdef TEMPERATURE_READER_WRITE_TO_SERIAL
        Serial.println("  Chip = DS18B20");
#endif
        type_s = 0;
        break;
    case 0x22:
#ifdef TEMPERATURE_READER_WRITE_TO_SERIAL
        Serial.println("  Chip = DS1822");
#endif
        type_s = 0;
        break;
    default:
#ifdef TEMPERATURE_READER_WRITE_TO_SERIAL
        Serial.println("Device is not a DS18x20 family device.");
#endif
        return;
    }

    dsRader->reset();
    dsRader->select(addr);
    dsRader->write(0x44, 1); // start conversion, with parasite power on at the end

    state = 2;
    stateSwitchTime = millis();
}
void TemperatureReader::handleState1()
{
    if (millis() - stateSwitchTime > pauseBeforeNextRead)
    {
        state = 0;
    }
}

void TemperatureReader::handleState2()
{
    if (millis() - stateSwitchTime > 1000)
    {
        state = 3;
    }
}
void TemperatureReader::handleState3()
{
    byte i;
    present = dsRader->reset();
    dsRader->select(addr);
    dsRader->write(0xBE); // Read Scratchpad

#ifdef TEMPERATURE_READER_WRITE_TO_SERIAL
    Serial.print("  Data = ");
    Serial.print(present, HEX);
    Serial.print(" ");
#endif
    for (i = 0; i < 9; i++)
    { // we need 9 bytes
        data[i] = dsRader->read();
#ifdef TEMPERATURE_READER_WRITE_TO_SERIAL
        Serial.print(data[i], HEX);
        Serial.print(" ");
#endif
    }

#ifdef TEMPERATURE_READER_WRITE_TO_SERIAL
    Serial.print(" CRC=");
    Serial.print(OneWire::crc8(data, 8), HEX);
    Serial.println();
#endif

    // Convert the data to actual temperature
    // because the result is a 16 bit signed integer, it should
    // be stored to an "int16_t" type, which is always 16 bits
    // even when compiled on a 32 bit processor.
    int16_t raw = (data[1] << 8) | data[0];
    if (type_s)
    {
        raw = raw << 3; // 9 bit resolution default
        if (data[7] == 0x10)
        {
            // "count remain" gives full 12 bit resolution
            raw = (raw & 0xFFF0) + 12 - data[6];
        }
    }
    else
    {
        byte cfg = (data[4] & 0x60);
        // at lower res, the low bits are undefined, so let's zero them
        if (cfg == 0x00)
            raw = raw & ~7; // 9 bit resolution, 93.75 ms
        else if (cfg == 0x20)
            raw = raw & ~3; // 10 bit res, 187.5 ms
        else if (cfg == 0x40)
            raw = raw & ~1; // 11 bit res, 375 ms
                            //// default is 12 bit resolution, 750 ms conversion time
    }
    celsius = (float)raw / 16.0;
    // fahrenheit = celsius * 1.8 + 32.0;
    // Serial.print("  Temperature = ");

#ifdef TEMPERATURE_READER_WRITE_TO_SERIAL
    Serial.print(celsius);
    Serial.println(" Celsius.");
#endif

    if (callback)
    {
        byte addr2[8];
        memccpy(addr2, addr, 8, sizeof(byte));
        callback(addr, celsius);
    }
    state = 0;
}

void TemperatureReader::loop()
{
    if (this->dsRader == NULL)
    {
        return;
    }

    switch (state)
    {
    case 0:
#ifdef TEMPERATURE_READER_WRITE_TO_SERIAL
        Serial.println();
        Serial.println("State 0");
#endif
        handleState0();
        break;
    case 1:
#ifdef TEMPERATURE_READER_WRITE_TO_SERIAL
        Serial.print(".");
#endif
        handleState1();
        break;
    case 2:
#ifdef TEMPERATURE_READER_WRITE_TO_SERIAL
        Serial.print(":");
#endif
        handleState2();
        break;
    case 3:
#ifdef TEMPERATURE_READER_WRITE_TO_SERIAL
        Serial.println();
        Serial.println("State 3");
#endif
        handleState3();
        break;
    default:
        state = 0;
        break;
    }
}

void TemperatureReader::begin(int pin, uint64_t pauseBeforeNextRead)
{
    this->dsRader = new OneWire(pin);
}