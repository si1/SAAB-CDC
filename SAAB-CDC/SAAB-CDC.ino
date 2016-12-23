/*
 BlueSaab v4.0

 A CD changer emulator for older SAAB cars with RN52 Bluetooth module by Microchip Technology Inc.
 
 Credits:

 Hardware design:           Seth Evans (http://bluesaab.blogspot.com)
 Initial code:              Seth Evans and Emil Fors
 CAN code:                  Igor Real (http://secuduino.blogspot.com)
 Information on SAAB I-Bus: Tomi Liljemark (http://pikkupossu.1g.fi/tomi/projects/i-bus/i-bus.html)
 RN52 handling:             based on code by Tim Otto (https://github.com/timotto/RN52lib)
 Additions/bug fixes:       Karlis Veilands, Girts Linde and Sam Thompson
*/


#include <Arduino.h>
#include <avr/wdt.h>
#include "CDC.h"
#include "RN52handler.h"
#include "Timer.h"

#define DEBUGMODE 0

CDChandler CDC;
Timer time;

int freeRam ()
{
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void setup() {
    wdt_disable(); // Allow delay loops greater than 15ms during setup.
    Serial.begin(9600);
    Serial.println(F("\"BlueSaab\""));
    Serial.print(F("Free SRAM: "));
    Serial.print(freeRam());
    Serial.println(F(" bytes"));
    Serial.println(F("Software version: v4.0"));
    BT.initialize();
    //Serial.println(F("Press H for Help"));
    CDC.openCanBus();
#if (DEBUGMODE==1)
    wdt_enable(WDTO_500MS); // give the loop time to do more serial diagnostic logging.
#else
    wdt_enable(WDTO_30MS);
#endif
}

void loop() {
#if (DEBUGMODE==1)
//    Serial.println(F("in loop()"));
#endif
    time.update();
    wdt_reset();
    CDC.handleCdcStatus();
    wdt_reset();
    BT.update();
    wdt_reset();
    BT.monitor_serial_input();
    wdt_reset();
}
