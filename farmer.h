#include <ctype.h>
#include "mbed.h"
#include "TMP36.h" //external library

#define WAIT_TIME .25 //define debounce time

#define TEMP_IN p20 //define pinout names
#define MOISTURE_IN p19
#define LIGHT_IN p18

#define MANUAL_ENABLE p14
#define MANUAL_RELAY p13
#define MANUAL_FAN p12
#define MANUAL_PUMP p11

#define FAN_OUT p21
#define RELAY_OUT p22
#define PUMP_OUT p23
#define MANUAL_LED p24

void read_sensors();

void toggle_fan();
void toggle_relay();
void toggle_pump();
void toggle_manual();

void print_menu();
void print_sensors();
void print_params();


