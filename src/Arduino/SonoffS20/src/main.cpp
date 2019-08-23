#include "env.cpp"
#include <Arduino.h>


const char *ssid = SECR_SSID;
const char *password = SECR_PASSWORD;
const char *mqtt_server = SECR_MQTT_SERVER;
const char *mqtt_user = SECR_MQTT_USER;
const char *mqtt_password = SECR_MQTT_PASSWORD;
const char *mqtt_client_id = MQTT_CLIENT_ID;
const char *mqtt_client_unique_id;
const char *mqtt_topic_subscribe;
const char *mqtt_topic_publish = CONNECTED_TOPIC; 
String mqttSubscibe;
String mqttButtonPressPublish;
String mqttClientUniqueueId;

int lastInputStatus;

unsigned long lastMqttReconectTime;
unsigned long lastInputStatusCounter;

bool lastTimeWifiConnected;

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

/// SONOFF SECTION BEGIN  !!! 

#define PUSH_BUTTON D6    // 0
#define RELAY_PLUS_LED D2 //  12
#define GREEN_LED D1      // 13

/// SONOFF SECTION END  !!! 

#define ON_STATE 0
#define OFF_STATE 1

#define GREEN_LED_ON 0
#define GREEN_LED_OFF 1

#define RELAY_ON 1
#define RELAY_OFF 0

#define PUSH_BUTTON_SWITCH_TIME_IN_MS 10
#define MQTT_RECONNECT_TIME_IN_MS 5000

int lastRelayStatus;

WiFiClient espClient;
PubSubClient client(espClient);
long int value = 0;

void setup_pins()
{
  delay(50);
  pinMode(PUSH_BUTTON, INPUT);
  pinMode(RELAY_PLUS_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  delay(50);
  digitalWrite(GREEN_LED, GREEN_LED_ON);
  lastInputStatus = digitalRead(PUSH_BUTTON);
  lastRelayStatus = RELAY_OFF;
  digitalWrite(RELAY_PLUS_LED, lastRelayStatus);
}

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  WiFi.setAutoConnect(true);

  if (WiFi.isConnected())
  {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("");
    lastTimeWifiConnected = true;
  }
  else
  {
    lastTimeWifiConnected = false;
  }

  randomSeed(micros());
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  char MD = (char)payload[0];
  // Switch on the LED if an 1 was received as first character
  if ('H' == MD)
  {
    lastRelayStatus = RELAY_ON;
    digitalWrite(RELAY_PLUS_LED, lastRelayStatus);
  }
  else if ('L' == MD)
  {
    lastRelayStatus = RELAY_OFF;
    digitalWrite(RELAY_PLUS_LED, lastRelayStatus);
  }
}

void handle_input_button()
{
  int button_status = digitalRead(PUSH_BUTTON);
  if (lastInputStatus != button_status)
  {
    unsigned long time = millis();
    if (time - lastInputStatusCounter > PUSH_BUTTON_SWITCH_TIME_IN_MS)
    {
      lastInputStatusCounter = millis();
      lastInputStatus = button_status;

      if (ON_STATE == lastInputStatus)
      {
        if (lastRelayStatus == RELAY_OFF)
        {
          lastRelayStatus = RELAY_ON;
          digitalWrite(RELAY_PLUS_LED, lastRelayStatus);
        }
        else
        {
          lastRelayStatus = RELAY_OFF;
          digitalWrite(RELAY_PLUS_LED, lastRelayStatus);
        }
      }

      if (client.connected())
      {
        client.publish(mqttButtonPressPublish.c_str(), ON_STATE == lastInputStatus ? "HIGH" : "LOW");
        Serial.println(mqttButtonPressPublish.c_str());
        Serial.println(ON_STATE == lastInputStatus ? "HIGH" : "LOW");
        Serial.println("published");
      }
    }
  }
  else
  {
    lastInputStatusCounter = millis();
  }
}

void reconnect()
{
  if (WiFi.isConnected())
  {
    if (!lastTimeWifiConnected)
    {
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      Serial.println("");
      lastTimeWifiConnected = true;
    }

    digitalWrite(GREEN_LED, GREEN_LED_OFF);
    // Loop until we're reconnected
    if (!client.connected())
    {
      if (millis() - lastMqttReconectTime > MQTT_RECONNECT_TIME_IN_MS)
      {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        // Attempt to connect
        if (client.connect(mqttClientUniqueueId.c_str(), mqtt_user, mqtt_password))
        {
          Serial.println();
          Serial.println("connected");
          // ... and resubscribe
          client.subscribe(mqttSubscibe.c_str());
          // Once connected, publish an announcement...
          Serial.println(mqttSubscibe.c_str());
          Serial.println("subscribed");
          client.publish(mqtt_topic_publish, mqtt_client_id);
          Serial.println(mqtt_topic_publish);
          Serial.println(mqtt_client_id);
          Serial.println("published");
          lastMqttReconectTime=0;
        }
        else
        {
          lastMqttReconectTime = millis();
          //lastMqttReconnectAtempt = gettime;
          Serial.println();
          Serial.print("failed, rc=");
          Serial.print(client.state());
          Serial.printf("try again in %d seconds\n",(int)(MQTT_RECONNECT_TIME_IN_MS/1000));
          Serial.println();
        }
      }
    }
  }
  else
  {
    lastMqttReconectTime = 0;
    lastTimeWifiConnected = false;
    digitalWrite(GREEN_LED, GREEN_LED_ON);
  }
}

void setup()
{
  delay(50);
  lastMqttReconectTime=0;
  lastInputStatusCounter=0;

  mqttClientUniqueueId = String(MQTT_CLIENT_ID);
  mqttClientUniqueueId += String("-");
  mqttClientUniqueueId += String(random(0xffff), HEX);

  mqttButtonPressPublish = String(BUTTON_TOPIC);
  mqttButtonPressPublish += String(MQTT_CLIENT_ID);

  mqttSubscibe = String(COMMAND_TOPIC);
  mqttSubscibe += String(MQTT_CLIENT_ID);

  setup_pins();
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  handle_input_button();
  client.loop();
}