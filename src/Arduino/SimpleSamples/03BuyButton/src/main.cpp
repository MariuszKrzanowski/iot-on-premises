#include "env.cpp"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char *ssid = SECR_SSID;
const char *password = SECR_PASSWORD;
const char *mqtt_server = SECR_MQTT_SERVER;
const char *mqtt_user = SECR_MQTT_USER;
const char *mqtt_password = SECR_MQTT_PASSWORD;
const char *mqtt_client_id = MQTT_CLIENT_ID;
const char *mqtt_client_unique_id;
const char *mqtt_topic_publish = PUBLISH_TOPIC;

String mqttBuyTopicPublish;
String mqttClientUniqueueId;
WiFiClient espClient;
PubSubClient client(espClient);
enum ApplicationStates : int
{
  WaitUntilSerialIsConnected = 1,
  ConnectToWiFi,
  ConnectToMqtt,
  PublishBuyAction,
  CleanupState,
  FinalState
};

ApplicationStates applicationState;

void waitState();
void connectToWiFi();
void connectToMqtt();
void publishBuyAction();
void finalState();
void cleanupState();

void setup()
{
  delay(50);
  randomSeed(micros());
  applicationState = ApplicationStates::WaitUntilSerialIsConnected;


  mqttClientUniqueueId = String(MQTT_CLIENT_ID);
  mqttClientUniqueueId += String("-");
  mqttClientUniqueueId += String(random(0xffff), HEX);

  mqttBuyTopicPublish = String(PUBLISH_TOPIC);
  Serial.begin(115200);
  Serial.println("In setup");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  delay(1000);  
  WiFi.begin(ssid, password); // PRESENTATION <=== Here we are setup WIFI 
  WiFi.setAutoConnect(true); 
  delay(50);
  client.setServer(mqtt_server, 1883); // PRESENTATION <=== Here we are setup MQTT server address and port
}


void loop()
{
  switch (applicationState)
  {
  case ApplicationStates::WaitUntilSerialIsConnected:
    waitState();
    break;
  case ApplicationStates::ConnectToWiFi:
    connectToWiFi();
    break;
  case ApplicationStates::ConnectToMqtt:
    connectToMqtt();
    break;
  case ApplicationStates::PublishBuyAction:
    publishBuyAction();
    break;
  case ApplicationStates::CleanupState:
    cleanupState();
    break;
  default:
    Serial.println("DEFAULT STATE BEGIN");
    delay(5000);
    Serial.println("DEFAULT STATE END");
    break;
  }
  delay(100);
}

void waitState()
{
  Serial.println("waitState BEGIN");
  delay(5000);
  Serial.println("waitState END");
  applicationState = ApplicationStates::ConnectToWiFi;
}

void connectToWiFi()
{
  Serial.println("connectToWiFi BEGIN");
  delay(1000);
  

  if (WiFi.isConnected())
  {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("");
    applicationState = ApplicationStates::ConnectToMqtt;
  }
  Serial.println("connectToWiFi END");
}

void connectToMqtt()
{
  Serial.println("connectToMqtt BEGIN");

  if (!client.connected())
  {
    // PRESENTATION <=== Here we are connecting to  MQTT server using client credentials
    if (client.connect(mqttClientUniqueueId.c_str(), mqtt_user, mqtt_password))  
    {
      delay(500);
    }
  }
  else
  {
      Serial.println();
      Serial.println("MQTT Client connected");
      applicationState = ApplicationStates::PublishBuyAction;
  }
  
  Serial.println("connectToMqtt END");
}

void publishBuyAction()
{
  Serial.println("publishBuyAction BEGIN");
  if (client.connected())
  {
    Serial.println(mqttBuyTopicPublish.c_str());
    Serial.println("BUY");
    // PRESENTATION <=== Here we are publishing MQTT message
    // TOPIC: shopping/actions/buy
    // PAYLOAD: BUY
    if (client.publish(mqttBuyTopicPublish.c_str(), "BUY"))
    {
      Serial.println("published");
      applicationState = ApplicationStates::CleanupState;
    }
  }
  else
  {
    applicationState = ApplicationStates::ConnectToMqtt;
  }

  Serial.println("publishBuyAction END");
}

void finalState()
{
  Serial.println("finalState BEGIN");
  delay(5000);

  Serial.println("finalState  END");
}

void cleanupState()
{
  Serial.println("cleanupState BEGIN");
  client.disconnect();
  WiFi.disconnect();
  applicationState=ApplicationStates::FinalState;
  Serial.println("cleanupState END");
}
