// This application reads MAC via RS-232 port and sends via MQTT.

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "secret.cpp"

#define MQTT_CLIENT_ID "sniffer1" // Remember to update it before firware upload !!!

const char *ssid = SECR_SSID;
const char *password = SECR_PASSWORD;
const char *mqtt_server = SECR_MQTT_SERVER;
const char *mqtt_user = SECR_MQTT_USER;
const char *mqtt_password = SECR_MQTT_PASSWORD;
const char *mqtt_client_id = MQTT_CLIENT_ID;
const char *mqtt_client_unique_id;
const char *mqtt_topic_subscribe;
const char *mqtt_topic_publish = "/sniff/sniffer1"; // Remember to update it before firware upload !!!

const int SNIFF_BUFFER_SIZE = 10;

String mqttClientUniqueueId;

WiFiClient espClient;
PubSubClient client(espClient);

int sniff_buffer_index_in = 0;
int sniff_buffer_index_out = 0;
String sniff_buffer[SNIFF_BUFFER_SIZE];

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
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
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    // Attempt to connect
    if (client.connect(mqttClientUniqueueId.c_str(), mqtt_user, mqtt_password))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void read_serial_line()
{
  while (true)
  {
    int peek = Serial.peek();
    if (-1 == peek)
    {
      return;
    }
    else
    {
      peek = Serial.read();
      char c = (char)peek;

      if (c == 'Z')
      {
        sniff_buffer[sniff_buffer_index_in] = String("Z");
        Serial.println();
        Serial.print("[Z]{");
        Serial.write(c);
      }
      else if (c == '\n')
      {
        if (((sniff_buffer_index_in + 1) % SNIFF_BUFFER_SIZE) != sniff_buffer_index_out)
        {
          sniff_buffer_index_in = (sniff_buffer_index_in + 1) % SNIFF_BUFFER_SIZE;
        }
        else
        {
          sniff_buffer[sniff_buffer_index_in] = String("Z");
        }

        Serial.print("}");
        Serial.println();
      }
      else if (c == '\r')
      {
        // ignore
      }
      else
      {
        sniff_buffer[sniff_buffer_index_in].concat(c);
        /* code */
        Serial.write(c);
      }
    }
    yield();
  }
}

void setup()
{
  Serial.begin(115200, SERIAL_8N1, SERIAL_FULL, 1);

  mqttClientUniqueueId = String(MQTT_CLIENT_ID);
  mqttClientUniqueueId += String("-");
  mqttClientUniqueueId += String(random(0xffff), HEX);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop()
{
  delay(1);
  read_serial_line();

  if (!client.connected())
  {
    reconnect();
  }
  if (client.connected())
  {
    if (sniff_buffer_index_in != sniff_buffer_index_out)
    {
      if ((sniff_buffer[sniff_buffer_index_out].length() > 22) && (sniff_buffer[sniff_buffer_index_out].length() < 29) && sniff_buffer[sniff_buffer_index_out].charAt(0) == 'Z')
      {
        client.publish(mqtt_topic_publish, sniff_buffer[sniff_buffer_index_out].c_str());
      }
      sniff_buffer_index_out = (sniff_buffer_index_out + 1) % SNIFF_BUFFER_SIZE;
    }
  }
  client.loop();
}
