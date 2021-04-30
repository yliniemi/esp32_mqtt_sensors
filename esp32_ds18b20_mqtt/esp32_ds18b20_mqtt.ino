#include "settings.h"             // THIS IS VERY IMPORTANT. CHANGE TESE SETTINGS HERE
#include <myCredentials.h>        // THIS ONE TOO. these is myCredentials.zip on the root of this repository. include it as a library and the edit the file with your onw ips and stuff

// #include <WiFi.h>
// #include <ESPmDNS.h>
// #include <WiFiUdp.h>
#include <EspMQTTClient.h>
#include "setupWifi.h"
#include "OTA.h"
#include "SerialOTA.h"

#include <OneWire.h>
#include <DallasTemperature.h>

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
//

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  
  setupWifi();
  
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  setupOTA();

  setupSerialOTA();

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
  float temperature = sensors.getTempCByIndex(0);
  if (temperature != -127) MQTTclient.publish(String(HOSTNAME) + "/temperature", String(temperature));          // the ds18b20 library gives a measurement of -127 when it doesn't get a reading
  Serial.println(String(HOSTNAME) + " temperature is " + temperature + " °C");
  SerialOTA.println(String(HOSTNAME) + " temperature is " + temperature + " °C");
  
  delay(1000);
}
