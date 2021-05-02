#define MAIN_INO
#include <DallasTemperature.h>
#include "settings.h"             // THIS IS VERY IMPORTANT. CHANGE TESE SETTINGS HERE
#include <myCredentials.h>        // THIS ONE TOO. these is myCredentials.zip on the root of this repository. include it as a library and the edit the file with your onw ips and stuff

#include "setupWifi.h"
#include "OTA.h"
#include "SerialOTA.h"
#include <EspMQTTClient.h>

OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

/* // This one lets EspMQTTclient control MQTT and WiFi
EspMQTTClient MQTTclient(
  //WIFI_SSID,
  //WIFI_PSK,
  MQTT_SERVER,      // MQTT Broker server ip
  MQTT_USERNAME,    // Can be omitted if not needed
  MQTT_PASSWORD,    // And this one too 
  HOSTNAME,         // Client name that uniquely identify your device
  1883              // default unencrypted port is 1883
);
*/

// This one is for letting it only handle MQTT and not WiFi
EspMQTTClient MQTTclient(
  MQTT_SERVER,        // MQTT Broker server ip
  1883,               // default unencrypted port is 1883
  // MQTT_USERNAME,   // Can be omitted if not needed
  // MQTT_PASSWORD,   // And this one too 
  HOSTNAME            // Client name that uniquely identify your device
);

void onConnectionEstablished()
{
  MQTTclient.subscribe(String("feed/") + HOSTNAME + "/#", [](const String & topic, const String & payload)
  {
    Serial.println("From wildcard topic: " + topic + ", payload: " + payload);
    SerialOTA.println("From wildcard topic: " + topic + ", payload: " + payload);
  });
  MQTTclient.publish(String(HOSTNAME) + "/alive", "I just woke up");
}

void printAddress(DeviceAddress address)
{ 
  Serial.print("{");
  SerialOTA.print("{");
  for (uint8_t i = 0; i < 8; i++)
  {
    Serial.print("0x");
    SerialOTA.print("0x");
    if (address[i] < 0x10)
    {
      Serial.print("0");
      SerialOTA.print("0");
    }
    Serial.print(address[i], HEX);
    SerialOTA.print(address[i], HEX);
    if (i < 7)
    {
      Serial.print(", ");
      SerialOTA.print(", ");
    }
  }
  Serial.println("}");
  SerialOTA.println("}");
}

void printSensorAddresses()
{
  Serial.println();
  Serial.print(sensors.getDeviceCount());
  Serial.println(" devices");
  Serial.println();
  SerialOTA.println();
  SerialOTA.print(sensors.getDeviceCount());
  SerialOTA.println(" devices");
  SerialOTA.println();
  
  Serial.println("Printing addresses:");
  SerialOTA.println("Printing addresses:");
  for (int i = 0;  i < sensors.getDeviceCount();  i++)
  {
    Serial.print("Sensor ");
    Serial.print(i);
    Serial.print(" : ");
    SerialOTA.print("Sensor ");
    SerialOTA.print(i);
    SerialOTA.print(" : ");
    DeviceAddress address;
    sensors.getAddress(address, i);
    printAddress(address);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  
  setupWifi();
  
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  setupOTA();

  setupSerialOTA();

  sensors.begin();
  
  // setupMQTT(HOSTNAME, MQTT_SERVER);
  MQTTclient.enableDebuggingMessages(); // Enable debugging messages sent to serial output

  // I had to do this trickery because EspMQTTclient -library doesn't support Strings for the lastwill for some reason
  String lastWillTopic = String(HOSTNAME) + "/lastwill";
  // char lastWillTopicChar[lastWillTopic.length() + 2];
  static char lastWillTopicChar[65];
  lastWillTopic.toCharArray(lastWillTopicChar, lastWillTopic.length() + 1);
  SerialOTA.println(lastWillTopicChar);
  Serial.println(lastWillTopicChar);
  MQTTclient.enableLastWillMessage(lastWillTopicChar, "What a world, what a world!");
}

void loop()
{
  reconnectToWifiIfNecessary();
  ArduinoOTA.handle();  
  SerialOTAhandle();
  MQTTclient.loop();
  
  static unsigned long previousTime = 0;
  if ((millis() - previousTime > 60000) || (millis() < previousTime))
  {
    printSensorAddresses();
    previousTime = millis();
  }
  
  static int loop_count = 0;
  loop_count++;
  Serial.println();
  Serial.print("loop count: ");
  Serial.println(loop_count);
  SerialOTA.println();
  SerialOTA.print("loop count: ");
  SerialOTA.println(loop_count);
  MQTTclient.publish(String(HOSTNAME) + "/loop_count", String(loop_count));
  
  sensors.requestTemperatures();
  
  #ifdef JUST_ONE_SENSOR
  float temperature = sensors.getTempCByIndex(0);
  if (temperature != -127) MQTTclient.publish(String(HOSTNAME) + "/temperature/onlyOne", String(temperature));          // the ds18b20 library gives a measurement of -127 when it doesn't get a reading
  Serial.println(String(HOSTNAME) + "/onlyOne temperature is " + temperature + " °C");
  SerialOTA.println(String(HOSTNAME) + "/onlyOne temperature is " + temperature + " °C");
  #endif

  #ifndef JUST_ONE_SENSOR
  for (int i = 0; sensorArray[i].name != 0; i++)
  {
    float temperature = sensors.getTempC(sensorArray[i].address);
    if (temperature != -127) MQTTclient.publish(String(HOSTNAME) + "/temperature/" + sensorArray[i].name, String(temperature));          // the ds18b20 library gives a measurement of -127 when it doesn't get a reading
    Serial.println(String(HOSTNAME) + "/" + sensorArray[i].name + " temperature is " + temperature + " °C");
    SerialOTA.println(String(HOSTNAME) + "/" + sensorArray[i].name + " temperature is " + temperature + " °C");
  }
  #endif
  
  delay(1000);
}
