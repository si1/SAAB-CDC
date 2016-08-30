/*
 BlueSaab v3.7

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

// Setup
void setup() {
    Serial.begin(9600);
    Serial.println("\"BlueSaab\" v3.7 - August 2016");
    BT.initialize();
    CDC.openCanBus();
    time.every(NODE_STATUS_TX_BASETIME, &sendCdcStatusOnTime,NULL);
    time.every(NODE_STATUS_TX_BASETIME, &sendDisplayRequestOnTime,NULL);
}

// Main loop
void loop() {
    time.update();
    CDC.handleCdcStatus();
    BT.update();
//    BT.monitor_serial_input(); // Switching the serial OFF for now; not sure why or if, but this seems to mess up things...
}
