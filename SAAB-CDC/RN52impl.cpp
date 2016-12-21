/*
 * Virtual C++ Class for RovingNetworks RN-52 Bluetooth modules
 * Copyright (C) 2013  Tim Otto
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Created by: Tim Otto
 * Created on: Jun 21, 2013
 * Modified by: Sam Thompson
 * Last modified on: Dec 16, 2016
 */

#include "RN52impl.h"
#include "RN52strings.h"

#define DEBUGMODE 0

/**
 * Reads the input (if any) from UART over software serial connection
 */

void RN52impl::readFromUART() {
    while (softSerial.available()) {
        char c = softSerial.read();
        fromUART(c);
//        if (currentCommand) {
        cmdResponseDeadline = millis() + cmdResponseTimeout;
//        }
    }
}


/**
 * Formats a message and writes it to UART over software serial
 */

void RN52impl::toUART(const char* c, int len){
    for(int i = 0; i < len; i++)
        softSerial.write(c[i]);
};

void RN52impl::fromSPP(const char* c, int len){
    // bytes received from phone via SPP
    
    // to send bytes back to the phone call rn52.toSPP
};

void RN52impl::setMode(Mode mode){
    if (mode == COMMAND) {
        digitalWrite(BT_CMD_PIN, LOW);
#if (DEBUGMODE==1)
        Serial.println(F("RN52: Set command mode."));
#endif
    } else if (mode == DATA) {
        digitalWrite(BT_CMD_PIN, HIGH);
#if (DEBUGMODE==1)
        Serial.println(F("RN52: Set data mode. "));
#endif
    }
};

void RN52impl::onError(int location, Error error){
    Serial.print(F("RN52 Error "));
    Serial.print(error);
    Serial.print(F(" at location: "));
    Serial.println(location);    
};

void RN52impl::onGPIO2() {
    queueCommand(RN52_CMD_QUERY);
}

void RN52impl::onProfileChange(BtProfile profile, bool connected) {
    switch(profile) {
        case A2DP:bt_a2dp = connected;
            if (connected && playing) {
                sendAVCRP(RN52::RN52driver::PLAYPAUSE);
            }
            break;
        case SPP:bt_spp = connected;break;
        case IAP:bt_iap = connected;break;
        case HFP:bt_hfp = connected;break;
    }
}

void RN52impl::update() {
    readFromUART();
    if (digitalRead(BT_EVENT_INDICATOR_PIN) == 0) {
        if ((millis() - lastEventIndicatorPinStateChange) > 100) {
            lastEventIndicatorPinStateChange = millis();
            onGPIO2();
#if (DEBUGMODE==1)
            Serial.println(F("Event Indicator Pin signalled.")); 
#endif
        }
    }
    if ( (long)( millis() - cmdResponseDeadline ) >= 0) {
        if (currentCommand) {
            // timed out. Bail on command if there is one, and reset
            cmdResponseDeadline = millis() + cmdResponseTimeout;
            Serial.println(F("Warning: Command timed out: "));
            abortCurrentCommand();
        }
    }
}

/**
 * Initializes Atmel pins and software serial for their initial state on startup
 */

void RN52impl::initialize() {

    // For hardware versions with 9600 baud locked, can set RN52 configuration after enabling it.
    bool configRN52postEnable = false;
  
    // Values used for "smoothing" analogRead() results for hardware revision check
    const int numOfReadings = 10;
    int sumOfReadings = 0;
    int hwRevisionCheckValue = 0;
    
    softSerial.begin(9600);
    
    for (int i = 0; i < numOfReadings; i++) {
        sumOfReadings = sumOfReadings + analogRead(HW_REV_CHK_PIN);
    }
    hwRevisionCheckValue = sumOfReadings / numOfReadings;
    
    // Initializing ATMEGA pins
    pinMode(BT_PWREN_PIN,OUTPUT);               // RN52 will not be restartable if rebooted with PWREN low.
    // No point in pulling low again.
    // According to RN52 DS70005120A p14 (section 2.5), cannot power down vreg.
    // Leaving high still allows pair timeout sleep.
    pinMode(BT_EVENT_INDICATOR_PIN,INPUT);
    pinMode(BT_CMD_PIN, OUTPUT);
    pinMode(BT_FACT_RST_PIN,OUTPUT);            // Some REALLY crazy stuff is going on if this pin is set as output and pulled low. Leave it alone! Trust me...
    pinMode(HW_REV_CHK_PIN,INPUT);              // We do an analogRead() on this pin to determine HW version of the module and take action accordingly
    pinMode(SN_XCEIVER_RS_PIN,OUTPUT);
    digitalWrite(BT_EVENT_INDICATOR_PIN,HIGH);  // Default state of GPIO2, per data sheet, is HIGH
    digitalWrite(BT_CMD_PIN,HIGH);              // Default state of GPIO9, per data sheet, is HIGH
    digitalWrite(BT_FACT_RST_PIN,HIGH);         // Default state of GPIO4, per data sheet, is LOW, but this is "voice command mode".

#if (DEBUGMODE==1)
    Serial.print(F("Revision check value: "));
    Serial.println(hwRevisionCheckValue);
#endif
    
    switch (hwRevisionCheckValue) {
        case 38 ... 52:                             // PCBs v3.3A, v4.1 or v4.2 (100K/5K Ohm network); TODO: make sure the correct resistors are soldered on!!!
            Serial.println(F("Hardware version: v3.3A/v4.1/v4.2"));
            digitalWrite(BT_PWREN_PIN,HIGH);
            break;
        case 83 ... 97:                             // PCB v4.3 (100K/10K Ohm network)
            Serial.println(F("Hardware version: v4.3"));
            digitalWrite(SN_XCEIVER_RS_PIN,LOW);    // This pin needs to be pulled low, otherwise SN65HVD251D CAN transciever goes into sleep mode
            digitalWrite(BT_PWREN_PIN,HIGH); // RN52 will not be restartable if rebooted with PWREN low. No point in pulling low again. According to RN52 DS70005120A p14 (section 2.5), cannot power down vreg.
            break;
        case 161 ... 175:                           // PCB v5.0 (100K/20K Ohm network)
            Serial.println(F("Hardware version: v5.0"));
            digitalWrite(SN_XCEIVER_RS_PIN,LOW);    // This pin needs to be pulled low, otherwise SN65HVD251D CAN transciever goes into sleep mode
            digitalWrite(BT_PWREN_PIN,HIGH); // RN52 will not be restartable if rebooted with PWREN low. No point in pulling low again. According to RN52 DS70005120A p14 (section 2.5), cannot power down vreg.
            configRN52postEnable = true;
            break;
        case 197 ... 213:                           // PCB v5.1 (100K/25K Ohm network)
            Serial.println(F("Hardware version: v5.1"));
            digitalWrite(SN_XCEIVER_RS_PIN,LOW);    // This pin needs to be pulled low, otherwise SN65HVD251D CAN transciever goes into sleep mode
            digitalWrite(BT_PWREN_PIN,HIGH); // RN52 will not be restartable if rebooted with PWREN low. No point in pulling low again. According to RN52 DS70005120A p14 (section 2.5), cannot power down vreg.
            // Need to add 555 stuff?
            configRN52postEnable = true;
            break;
        default:                                    // PCB revision is older than v3.3A; PWREN is hardwired to 3v3; no other action needs to be taken
            Serial.println(F("Hardware version: Legacy"));
            break;
    }    
    // Configuring RN52
    if (configRN52postEnable) {
        Serial.println(F("Configuring RN52... "));
        set_discovery_mask();
        set_connection_mask();
        set_cod();
        set_device_name();
        set_extended_features();
        set_max_volume();
        set_pair_timeout();
        reboot();
        queueCommand("G%\r"); // Was D.
        processCmdQueue();
        Serial.println(F("Configured RN52"));
    }
}

void RN52impl::processCmdQueue() {
#if (DEBUGMODE==1)
    Serial.println(F("Processing cmd queue."));
#endif
    do {
        update();
    } while (getQueueSize() || currentCommand != NULL); //FIXME: fails if only 1 cmd in the queue to start.
}
