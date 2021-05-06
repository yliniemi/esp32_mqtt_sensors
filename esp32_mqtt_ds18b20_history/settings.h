#ifndef SETTINGS_H
#define SETTINGS_H

#define HOSTNAME "myTemps2"             // replace this with the name for this particular device. everyone deserves a unique name
#define oneWireBus 17
// #define JUST_ONE_SENSOR              // uncomment this if you are in a hurry and don't want to manually list sensor addresses in sensorArray
#define USING_SERIALOTA                 // uncomment this if you are not using SerialOTA

#ifdef MAIN_INO
typedef struct sensor
{
  DeviceAddress address;
  char * name;
};

// here you define the address of your sensors
// they are printed to Serial and SerialOTA once a minute
// what follows the address is it's name
// the name is used in topic like this HOSTNAME/name. they don't have to be numbers.
struct sensor sensorArray[] = {
  {{0x28, 0x00, 0x2A, 0x0F, 0x3B, 0x47, 0x06, 0xF6}, "0"},
  {{0x28, 0x00, 0x1A, 0x5A, 0x3B, 0x47, 0x04, 0x97}, "1"},
  {{0x28, 0x00, 0x35, 0x62, 0x3B, 0x47, 0x04, 0x54}, "2"},
  {{0x28, 0xFF, 0x64, 0x1E, 0x0E, 0x60, 0xA6, 0xB7}, "3"},
  {{0x28, 0xFF, 0x64, 0x1E, 0x0E, 0x73, 0x7A, 0xFA}, "4"},
  {0, 0}};                                                        // the 0 is important this is how we know that we have reached the end of the array
#endif

#endif
