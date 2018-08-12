# 1 "c:\\users\\ralf\\appdata\\local\\temp\\tmptdewty"
#include <Arduino.h>
# 1 "D:/Projects/weatherman-embedded/src/HelloServer.ino"
#include <ESP8266WiFi.h>

#include <WiFiClient.h>

#include <ESP8266WebServer.h>

#include <ESP8266mDNS.h>

#include "DHT.h"

#include <PubSubClient.h>



using namespace std;



DHT dht;



char *ssid = "keller2";

const char *password = "1234321234321";

char *mqtt_server = "192.168.43.211";



WiFiClient espClient;

PubSubClient client(espClient);

long lastMsg = 0;

char msg[50];

int value = 0;



ESP8266WebServer server(80);



const int led = 13;

const int d1 = 5;

const int d2 = 4;

const int d3 = 0;

const int d4 = 2;

const int d5 = 14;

const int d6 = 12;

const int d7 = 13;

const int d8 = 15;
void handleRoot();
void handleNotFound();
void setupWifi();
void setupMDNS(void);
void setupDHT(void);
void reconnect();
void handleSwitch(byte *payload);
void printMessage(char *topic, byte *payload, unsigned int length);
void mqttCallback(char *topic, byte *payload, unsigned int length);
void setupMQTT(void);
void setupWebServer(void);
void setup(void);
void loop(void);
#line 67 "D:/Projects/weatherman-embedded/src/HelloServer.ino"
void handleRoot()

{

  digitalWrite(led, 1);



  float humidity = dht.getHumidity();

  float temperature = dht.getTemperature();



  server.send(200, "text/plain", "hello from esp8266! Humidity: " + String(humidity, 2) + "\% Temperature: " + String(temperature, 2) + "°C");



  digitalWrite(led, 0);

}



void handleNotFound()

{

  digitalWrite(led, 1);

  String message = "File Not Found\n\n";

  message += "URI: ";

  message += server.uri();

  message += "\nMethod: ";

  message += (server.method() == HTTP_GET) ? "GET" : "POST";

  message += "\nArguments: ";

  message += server.args();

  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++)

  {

    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";

  }

  server.send(404, "text/plain", message);

  digitalWrite(led, 0);

}



void setupWifi()

{

  delay(10);



  Serial.println();

  Serial.print("Connecting to ");

  Serial.println(ssid);



  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, password);



  const int retryMax = 10;

  int retryCount = 0;



  while (WiFi.status() != WL_CONNECTED)

  {

    delay(1000);

    Serial.print(".");



    if (retryCount >= retryMax)

    {

      ssid = "keller";

      mqtt_server = "192.168.178.112";



      if (retryCount == retryMax)

      {

        WiFi.begin(ssid, password);

      }

    }



    retryCount++;

  }



  randomSeed(micros());



  Serial.println("");

  Serial.println("WiFi connected");

  Serial.println("IP address: ");

  Serial.println(WiFi.localIP());

}



void setupMDNS(void)

{

  if (MDNS.begin("esp8266"))

  {

    Serial.println("MDNS responder started");

  }

}



void setupDHT(void)

{

  dht.setup(2);

}



void reconnect()

{



  while (!client.connected())

  {

    Serial.print("Attempting MQTT connection...");



    String clientId = "ESP82-sensor-";

    clientId += String(random(0xffff), HEX);



    if (client.connect(clientId.c_str()))

    {

      Serial.println("connected");



      client.publish("sensornode/1/ip", WiFi.localIP().toString().c_str());



    }

    else

    {

      Serial.print("failed, rc=");

      Serial.print(client.state());

      Serial.println(" try again in 5 seconds");



      delay(5000);

    }

  }

}



void handleSwitch(byte *payload)

{

  Serial.println((char)payload[0]);



  if ((char)payload[0] == '1')

  {

    Serial.print("set pin d2 high");

    digitalWrite(d2, 1);

  }

  else if ((char)payload[0] == '0')

  {

    Serial.print("set pin d2 low");

    digitalWrite(d2, 0);

  }

}



void printMessage(char *topic, byte *payload, unsigned int length)

{

  Serial.print("Message arrived [");

  Serial.print(topic);

  Serial.print("] ");

  for (int i = 0; i < length; i++)

  {

    Serial.print((char)payload[i]);

  }

  Serial.println();

}



void mqttCallback(char *topic, byte *payload, unsigned int length)

{

  printMessage(topic, payload, length);



  if (strcmp(topic, "sensornode/1/switch") == 0)

  {

    handleSwitch(payload);

  }





  if ((char)payload[0] == '1')

  {

    digitalWrite(BUILTIN_LED, LOW);





  }

  else

  {

    digitalWrite(BUILTIN_LED, HIGH);

  }

}



void setupMQTT(void)

{

  client.setServer(mqtt_server, 1883);

  client.setCallback(mqttCallback);

  delay(500);



  if (!client.subscribe("sensornode/1/+"))

  {

    Serial.println("Subscription failed.");

  }

}



void setupWebServer(void)

{

  server.on("/", handleRoot);



  server.on("/inline", []() {

    server.send(200, "text/plain", "this works as well");

  });



  server.onNotFound(handleNotFound);



  server.begin();

  Serial.println("HTTP server started");

}



void setup(void)

{

  pinMode(led, OUTPUT);

  pinMode(d2, OUTPUT);

  digitalWrite(led, 0);

  Serial.begin(115200);



  setupWifi();



  setupMDNS();



  setupDHT();



  setupMQTT();



  setupWebServer();

}



const char *StringToChar(String string)

{

  std::vector<char> writable(string.begin(), string.end());

  writable.push_back('\0');



  return &*writable.begin();

}



const char *floatToChar(float number)

{

  String string = String(number, 2);

  return StringToChar(string);

}



void loop(void)

{

  if (!client.connected())

  {

    reconnect();

  }

  client.loop();



  long now = millis();



  if (now - lastMsg > 10000)

  {

    lastMsg = now;

    ++value;

    snprintf(msg, 75, "hello world #%ld", value);

    Serial.print("Publish message: ");

    Serial.println(msg);

    client.publish("sensornode/1/log", msg);



    float temperature = dht.getTemperature();

    float humidity = dht.getHumidity();



    client.publish("sensornode/1/temperature", floatToChar(temperature));

    client.publish("sensornode/1/humidity", floatToChar(humidity));

  }



  server.handleClient();

}