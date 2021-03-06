#include "settings.h"             // THIS IS VERY IMPORTANT. CHANGE TESE SETTINGS HERE
#include <myCredentials.h>        // THIS ONE TOO. these is myCredentials.zip on the root of this repository. include it as a library and the edit the file with your onw ips and stuff

#include "setupWifi.h"
#include "OTA.h"

#ifdef USING_SERIALOTA
#include "SerialOTA.h"
#endif

#include <EspMQTTClient.h>
#include <DHT.h>                  // dht library by adafruit

DHT dht(DHTPIN, DHTTYPE);                           // Alustus

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

  Serial.println("DHT11 testi!");
  dht.begin();

  // setupMQTT(HOSTNAME, MQTT_SERVER);
  MQTTclient.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  
  // I had to do this trickery because EspMQTTclient -library doesn't support Strings for the lastwill for some reason
  String lastWillTopic = String(HOSTNAME) + "/lastwill";
  //char lastWillTopicChar[lastWillTopic.length() + 1];
  static char lastWillTopicChar[65];
  lastWillTopic.toCharArray(lastWillTopicChar, lastWillTopic.length());
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

  //Kosteus ja l??mp????
  dht.readTemperature();                                                //L??mp??tila (celsius oletuksena)
  dht.readHumidity();                                                   //Kosteusarvo

  //Kosteus ja l??mp????
  float temperature = dht.readTemperature(0);                           //Lue l??mp??tila (celsius oletuksena)
  if (isnan(temperature))
  {
    Serial.println("L??mp??tilan lukeminen ep??onnistui!");
    SerialOTA.println("L??mp??tilan lukeminen ep??onnistui!");
  }
  else
  {
    MQTTclient.publish(String(HOSTNAME) + "/temperature", String(temperature));               //L??hett???? tiedon node-redille, MQTT Broker Mosquitto nimell??: "bedroom/temperature"
    Serial.println(String(HOSTNAME) + " l??mp??tila on " + temperature + " ??C");    //tulostaa juuri luettu l??mp??tila serialmonitorille
    SerialOTA.println(String(HOSTNAME) + " l??mp??tila on " + temperature + " ??C"); //tulostaa juuri luettu l??mp??tila langattomasti    
  }
  
  float humidity = dht.readHumidity(0);                                 //Lue kosteus
  if (isnan(humidity))
  {
    Serial.println("Kosteuden lukeminen ep??onnistui!");
    SerialOTA.println("Kosteuden lukeminen ep??onnistui!");
  }
  else
  {
    MQTTclient.publish(String(HOSTNAME) + "/humidity", String(humidity));                  //L??hett???? tiedon node-redille, MQTT Broker Mosquitto nimell??: "bedroom/humidity"
    Serial.println(String(HOSTNAME) + " kosteus on " + humidity + " %");       //tulostaa juuri luettu kosteusarvo serialmonitorille
    SerialOTA.println(String(HOSTNAME) + " kosteus on " + humidity + " %");    //tulostaa juuri luettu kosteusarvo langattomasti
  }
  
  delay(1000);
}
