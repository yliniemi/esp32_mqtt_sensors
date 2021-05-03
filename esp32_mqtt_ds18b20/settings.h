#ifndef SETTINGS_H
#define SETTINGS_H

#define HOSTNAME "myTemps1"             // replace this with the name for this particular device. everyone deserves a unique name
#define oneWireBus 18
// #define JUST_ONE_SENSOR             // uncomment this if you are in a hurry and don't want to manually list sensor addresses in sensorArray
#define USING_SERIALOTA           // uncomment this if you are not using SerialOTA

#ifdef MAIN_INO
typedef struct sensor
{
  DeviceAddress address;
  char * name;
};

// here you define the address of your sensors
// they are printed to Serial and SerialOTA once a minute
// what follows the address is it's name
// the name is used in topic like this HOSTNAME/name
struct sensor sensorArray[] = {
  {{0x28, 0xAA, 0xC6, 0xC5, 0x53, 0x14, 0x01, 0x9D}, "3m"},
  {{0x28, 0xFF, 0xBF, 0x48, 0x81, 0x17, 0x05, 0xE0}, "1m"},
  {0, 0}};                                                        // the 0 is important this is how we know that we have reached the end of the array
#endif

#endif
