#include "../../.secret/secret.cpp"

#ifndef SECR_DATA_DEFINED
#warning "Update data in this section"
#define SECR_SSID "iotdemo"
#define SECR_PASSWORD "iotpasswd"
#define SECR_MQTT_SERVER "192.168.0.10"
#define SECR_MQTT_USER "UserMqtt"
#define SECR_MQTT_PASSWORD "PasswordMqtt"
#endif

#define CONNECTED_TOPIC "/socket/connected" // Remember to update it before firware upload !!!
#define COMMAND_TOPIC "/socket/command/"
#define BUTTON_TOPIC "/socket/button/"
#define MQTT_CLIENT_ID "sonoff" // Remember to update it before firware upload !!!
