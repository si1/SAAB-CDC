/*
 BlueSaab v3.7

 A CD changer emulator for older SAAB cars with RN52 bluetooth module.
 
 Credits:

 Hardware design: Seth Evans (http://bluesaab.blogspot.com)
 Initial CDC code: Seth Evans and Emil Fors
 CAN code: Igor Real (http://secuduino.blogspot.com)
 Information on SAAB I-Bus: Tomi Liljemark (http://pikkupossu.1g.fi/tomi/projects/i-bus/i-bus.html)
 RN52 handling:  based on code by Tim Otto
 Additions/bug fixes: Karlis Veilands and Girts Linde
*/


#include <Arduino.h>
#include "CDC.h"
#include "Globals.h"
#include "RN52handler.h"
#include "Timer.h"

CDChandler CDC;
Timer time;


// Setup
void setup() {
    Serial.begin(BAUDRATE);
    Serial.println("\"BlueSaab\" v3.7 - August 2016");
    BT.initialize();
    CDC.openCanBus();
    time.every(CDC_STATUS_TX_TIME, &sendCdcStatusOnTime,NULL);
}

// Main loop
void loop() {
    time.update();
    CDC.handleCdcStatus();
    BT.update();
    if (DEBUGMODE==1) {
        BT.monitor_serial_input();
    }
}
