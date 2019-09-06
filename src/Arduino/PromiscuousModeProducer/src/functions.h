// WI-FI sniffer uses modified files from 'Wi-Fi Sniffer as a Human detector repository'.
// See: https://github.com/SensorsIot/Wi-Fi-Sniffer-as-a-Human-detector

#include "./structures.h"
#include <ESP8266WiFi.h>
// Expose Espressif SDK functionality

#ifndef MINRSSI
#define MINRSSI -100
#endif

#ifndef SERIAL_WRITE_BOTH
#define SERIAL_WRITE_BOTH true
#endif

#ifndef SHOW_BEACON
#define SHOW_BEACON true
#endif

#ifndef SHOW_CLIENT
#define SHOW_CLIENT true
#endif

extern "C"
{
#include "user_interface.h"
    typedef void (*freedom_outside_cb_t)(uint8 status);
    int wifi_register_send_pkt_freedom_cb(freedom_outside_cb_t cb);
    void wifi_unregister_send_pkt_freedom_cb(void);
    int wifi_send_pkt_freedom(uint8 *buf, int len, bool sys_seq);
}

#define MAX_APS_TRACKED 100
#define MAX_CLIENTS_TRACKED 200

beaconinfo aps_known[MAX_APS_TRACKED]; // Array to save MACs of known APs
int aps_known_count = 0;               // Number of known APs
int nothing_new = 0;
clientinfo clients_known[MAX_CLIENTS_TRACKED]; // Array to save MACs of known CLIENTs
int clients_known_count = 0;                   // Number of known CLIENTs

String formatMac1(uint8_t mac[ETH_MAC_LEN])
{
    String hi = "";
    for (int i = 0; i < ETH_MAC_LEN; i++)
    {
        if (mac[i] < 16)
            hi = hi + "0" + String(mac[i], HEX);
        else
            hi = hi + String(mac[i], HEX);
        if (i < 5)
            hi = hi + ":";
    }
    return hi;
}

int register_beacon(beaconinfo beacon)
{
    // return 1; //TODO: MUKR -- workaround to eliminate beacons
    int known = 0; // Clear known flag
    for (int u = 0; u < aps_known_count; u++)
    {
        if (!memcmp(aps_known[u].bssid, beacon.bssid, ETH_MAC_LEN))
        {
            aps_known[u].lastDiscoveredTime = millis();
            aps_known[u].rssi = beacon.rssi;
            known = 1;
            break;
        } // AP known => Set known flag
    }
    if (!known && (beacon.err == 0)) // AP is NEW, copy MAC to array and return it
    {
        beacon.lastDiscoveredTime = millis();
        memcpy(&aps_known[aps_known_count], &beacon, sizeof(beacon));

        aps_known_count++;

        if ((unsigned int)aps_known_count >=
            sizeof(aps_known) / sizeof(aps_known[0]))
        {
            if (SERIAL_WRITE_BOTH)
            {
                Serial.printf("exceeded max aps_known\n");
            }
            aps_known_count = 0;
        }
    }
    return known;
}

int register_client(clientinfo &ci)
{
    int known = 0; // Clear known flag
    for (int u = 0; u < clients_known_count; u++)
    {
        if (!memcmp(clients_known[u].station, ci.station, ETH_MAC_LEN))
        {
            clients_known[u].lastDiscoveredTime = millis();
            clients_known[u].rssi = ci.rssi;
            known = 1;
            break;
        }
    }

    //Uncomment the line below to disable collection of probe requests from randomised MAC's
    //if (ci.channel == -2) known = 1; // This will disable collection of probe requests from randomised MAC's

    if (!known)
    {
        ci.lastDiscoveredTime = millis();
        // search for Assigned AP
        for (int u = 0; u < aps_known_count; u++)
        {
            if (!memcmp(aps_known[u].bssid, ci.bssid, ETH_MAC_LEN))
            {
                ci.channel = aps_known[u].channel;
                break;
            }
        }
        if (ci.channel != 0)
        {
            memcpy(&clients_known[clients_known_count], &ci, sizeof(ci));

            clients_known_count++;
        }

        if ((unsigned int)clients_known_count >=
            sizeof(clients_known) / sizeof(clients_known[0]))
        {
            if (SERIAL_WRITE_BOTH)
            {
                Serial.printf("exceeded max clients_known\n");
            }
            clients_known_count = 0;
        }
    }
    return known;
}

void print_beacon(beaconinfo beacon)
{
    String hi = "";
    if (beacon.err != 0)
    {
        if (SERIAL_WRITE_BOTH)
        {
            Serial.printf("BEACON ERR: (%d)  ", beacon.err);
        }
    }
    else
    {
        if (beacon.rssi > MINRSSI)
        {

            if (SERIAL_WRITE_BOTH && SHOW_BEACON)
            {
                String fm = formatMac1(beacon.bssid);
                
                Serial.printf(" BEACON: <=============== [%32s]  ", beacon.ssid);
                Serial.print(formatMac1(beacon.bssid));
                Serial.printf("   %2d", beacon.channel);
                Serial.printf("   %4d\r\n", beacon.rssi);
            }
            if (SHOW_BEACON)
            {
                Serial.print("ZB:");
                Serial.print(formatMac1(beacon.bssid));
                Serial.printf(":%d", beacon.channel);
                Serial.printf("=%d\r\n", beacon.rssi);
            }
        }
    }
}

void print_client(clientinfo ci)
{

    int u = 0;
    int known = 0; // Clear known flag
    if (ci.err != 0)
    {
        // nothing
    }
    else
    {
        if (ci.rssi > MINRSSI)
        {
            if (SERIAL_WRITE_BOTH && SHOW_CLIENT)
            {
                String fm = formatMac1(ci.station);

                Serial.printf("CLIENT: ");
                Serial.print(formatMac1(ci.station)); //Mac of device

                Serial.print(formatMac1(ci.ap));   // Mac of connected AP
                Serial.printf(":%3d", ci.channel); //used channel
                Serial.printf(":%4d\r\n", ci.rssi);
            }
            if (SHOW_CLIENT)
            {
                Serial.print("ZC:");
                Serial.print(formatMac1(ci.station));
                Serial.printf(":%d", ci.channel);
                Serial.printf("=%d\r\n", ci.rssi);
            }
        }
        //    }
    }
}

void promisc_cb(uint8_t *buf, uint16_t len)
{
    int i = 0;
    uint16_t seq_n_new = 0;
    if (len == 12)
    {
        struct RxControl *sniffer = (struct RxControl *)buf;
    }
    else if (len == 128)
    {
        struct sniffer_buf2 *sniffer = (struct sniffer_buf2 *)buf;
        if ((sniffer->buf[0] == 0x80))
        {
            struct beaconinfo beacon = parse_beacon(sniffer->buf, 112, sniffer->rx_ctrl.rssi);
            if (register_beacon(beacon) == 0)
            {
                print_beacon(beacon);
                nothing_new = 0;
            }
        }
        else if ((sniffer->buf[0] == 0x40))
        {
            struct clientinfo ci = parse_probe(sniffer->buf, 36, sniffer->rx_ctrl.rssi);
            //if (memcmp(ci.bssid, ci.station, ETH_MAC_LEN)) {
            if (register_client(ci) == 0)
            {
                print_client(ci);
                nothing_new = 0;
            }
            //}
        }
    }
    else
    {
        struct sniffer_buf *sniffer = (struct sniffer_buf *)buf;
        //Is data or QOS?
        if ((sniffer->buf[0] == 0x08) || (sniffer->buf[0] == 0x88))
        {
            struct clientinfo ci = parse_data(sniffer->buf, 36, sniffer->rx_ctrl.rssi, sniffer->rx_ctrl.channel);
            if (memcmp(ci.bssid, ci.station, ETH_MAC_LEN))
            {
                if (register_client(ci) == 0)
                {
                    print_client(ci);
                    nothing_new = 0;
                }
            }
        }
    }
}
