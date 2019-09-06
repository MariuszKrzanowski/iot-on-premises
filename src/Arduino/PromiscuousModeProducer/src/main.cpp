// This WI-FI sniffer sends found MAC addresess via RS-232 port.

#include <Arduino.h>
#include <ESP8266WiFi.h>

#define disable 0
#define enable 1
#define MINRSSI -100
#define SHOW_BEACON false
#define SHOW_CLIENT true
#define SERIAL_WRITE_BOTH false

#include "functions.h"

unsigned int channel = 1;

void setup()
{
  delay(50);
  Serial.begin(115200, SERIAL_8N1, SERIAL_FULL, 1);
  Serial.println();
  if (SERIAL_WRITE_BOTH)
  {
    Serial.printf("\n\nSDK version:%s\n\r", system_get_sdk_version());
  }
  Serial.println();
  delay(50);
  Serial.println("READY");
  wifi_set_opmode(STATION_MODE); // Promiscuous works only with station mode
  wifi_set_channel(-1);
  wifi_promiscuous_enable(disable);
  wifi_set_promiscuous_rx_cb(promisc_cb); // Set up promiscuous callback
  wifi_promiscuous_enable(enable);
}

int channel_timout;

void loop()
{
  channel = 0;
  wifi_set_channel(channel);
  channel++;

  while (true)
  {
    nothing_new++; // Array is not finite, check bounds and adjust if required
    channel_timout++;
    if (nothing_new > 200 || channel_timout > 300)
    {
      clients_known_count = 0;
      aps_known_count = 0;
      nothing_new = 0;
      channel_timout = 0;
      channel++;
      if (channel == 15)
        break; // Only scan channels 1 to 14
      wifi_set_channel(channel);
    }
    delay(1); // critical processing timeslice for NONOS SDK! No delay(0) yield()
  }
}