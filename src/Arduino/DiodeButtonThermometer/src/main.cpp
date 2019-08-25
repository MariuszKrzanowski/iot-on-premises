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
String mqttTemperaturePublish;
String mqttClientUniqueueId;

int lastInputStatus;

unsigned long lastMqttReconectTime;
unsigned long lastInputStatusCounter;

bool lastTimeWifiConnected;

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "TemperatureReader.h"

#define PUSH_BUTTON D3
#define LIGHT_LED D1
#define ONE_WIRE_PIN D7

#define BUTTON_DOWN 0
#define BUTTON_UP 1

#define LIGHT_LED_ON 1
#define LIGHT_LED_OFF 0

#define PUSH_BUTTON_SWITCH_TIME_IN_MS 10
#define MQTT_RECONNECT_TIME_IN_MS 5000

int lastLightStatus;

WiFiClient espClient;
PubSubClient client(espClient);
TemperatureReader tempartureReader;

long int value = 0;

void setup_pins()
{
  delay(50);
  pinMode(PUSH_BUTTON, INPUT);
  pinMode(LIGHT_LED, OUTPUT);
  delay(50);
  digitalWrite(LIGHT_LED, LIGHT_LED_OFF);
  lastInputStatus = digitalRead(PUSH_BUTTON);
  lastLightStatus = LIGHT_LED_OFF;
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

void temparture(byte *sensorAddress, float celsius)
{
  Serial.println("Callback");
  byte i;
  Serial.print("ROM =");
  for (i = 0; i < 8; i++)
  {
    Serial.write(' ');
    Serial.print(sensorAddress[i], HEX);
  }
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.println(" Celsius.");



  if (client.connected())
  {
    String strCelsius=String(celsius,2);
    client.publish(mqttTemperaturePublish.c_str(), strCelsius.c_str());
    Serial.println(mqttTemperaturePublish.c_str());
    Serial.println(strCelsius.c_str());
    Serial.println("published");
  }
}

void setup_temparture_reader()
{
  tempartureReader.setCallback(temparture);
  tempartureReader.begin(ONE_WIRE_PIN, 8000);
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
    lastLightStatus = LIGHT_LED_ON;
    digitalWrite(LIGHT_LED, lastLightStatus);
  }
  else if ('L' == MD)
  {
    lastLightStatus = LIGHT_LED_OFF;
    digitalWrite(LIGHT_LED, lastLightStatus);
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

      if (BUTTON_DOWN == lastInputStatus)
      {
        if (lastLightStatus == LIGHT_LED_OFF)
        {
          lastLightStatus = LIGHT_LED_ON;
          digitalWrite(LIGHT_LED, lastLightStatus);
        }
        else
        {
          lastLightStatus = LIGHT_LED_OFF;
          digitalWrite(LIGHT_LED, lastLightStatus);
        }
      }

      if (client.connected())
      {
        client.publish(mqttButtonPressPublish.c_str(), BUTTON_DOWN == lastInputStatus ? "HIGH" : "LOW");
        Serial.println(mqttButtonPressPublish.c_str());
        Serial.println(BUTTON_DOWN == lastInputStatus ? "HIGH" : "LOW");
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

    digitalWrite(LIGHT_LED, LIGHT_LED_OFF);
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
          lastMqttReconectTime = 0;
        }
        else
        {
          lastMqttReconectTime = millis();
          //lastMqttReconnectAtempt = gettime;
          Serial.println();
          Serial.print("failed, rc=");
          Serial.print(client.state());
          Serial.printf("try again in %d seconds\n", (int)(MQTT_RECONNECT_TIME_IN_MS / 1000));
          Serial.println();
        }
      }
    }
  }
  else
  {
    lastMqttReconectTime = 0;
    lastTimeWifiConnected = false;
    digitalWrite(LIGHT_LED, LIGHT_LED_ON);
  }
}

void setup()
{
  delay(50);
  lastMqttReconectTime = 0;
  lastInputStatusCounter = 0;

  mqttClientUniqueueId = String(MQTT_CLIENT_ID);
  mqttClientUniqueueId += String("-");
  mqttClientUniqueueId += String(random(0xffff), HEX);

  mqttButtonPressPublish = String(BUTTON_TOPIC);
  mqttButtonPressPublish += String(MQTT_CLIENT_ID);

  mqttTemperaturePublish = String(TEMPERATURE_TOPIC);
  mqttTemperaturePublish += String(MQTT_CLIENT_ID);

  mqttSubscibe = String(COMMAND_TOPIC);
  mqttSubscibe += String(MQTT_CLIENT_ID);

  setup_pins();
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  setup_temparture_reader();
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  handle_input_button();
  client.loop();
  tempartureReader.loop();
  delay(5); // give a little rest for CPU. Less heat is produced.
}