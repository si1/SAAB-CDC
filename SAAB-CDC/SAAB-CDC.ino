/*
 BlueSaab v4.0 Beta

 A CD changer emulator for older SAAB cars with RN52 Bluetooth module by Microchip Technology Inc.
 
 Credits:

 Hardware design:           Seth Evans (http://bluesaab.blogspot.com)
 Initial code:              Seth Evans and Emil Fors
 CAN code:                  Igor Real (http://secuduino.blogspot.com)
 Information on SAAB I-Bus: Tomi Liljemark (http://pikkupossu.1g.fi/tomi/projects/i-bus/i-bus.html)
 RN52 handling:             based on code by Tim Otto
 Additions/bug fixes:       Karlis Veilands and Girts Linde
*/


#include <Arduino.h>
#include "CDC.h"
#include "RN52handler.h"
#include "Timer.h"

CDChandler CDC;
Timer time;

int freeRam ()
{
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void setup() {
    Serial.begin(9600);
    Serial.println("\"BlueSaab\"");
    Serial.print(F("SRAM utilization: "));
    Serial.println(freeRam());
    Serial.println("Software version: v4.0 Beta");
    BT.initialize();
    CDC.openCanBus();
}

void loop() {
    time.update();
    CDC.handleCdcStatus();
    BT.update();
    BT.monitor_serial_input();
}
