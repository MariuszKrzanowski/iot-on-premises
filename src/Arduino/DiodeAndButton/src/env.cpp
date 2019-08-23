#include "../../.secret/secret.cpp"

#ifndef SECR_DATA_DEFINED
#warning "Update data in this section"
#define SECR_SSID "iotdemo"
#define SECR_PASSWORD "iotpasswd"
#define SECR_MQTT_SERVER "192.168.0.10"
#define SECR_MQTT_USER "UserMqtt"
#define SECR_MQTT_PASSWORD "PasswordMqtt"
#endif

#define CONNECTED_TOPIC "/light/connected" // Remember to update it before firware upload !!!
#define COMMAND_TOPIC "/light/command/"
#define BUTTON_TOPIC "/light/button/"
#define MQTT_CLIENT_ID "lightGreen" // Remember to update it before firware upload !!!

// lightGreen
// lightRed
// lightYellow
