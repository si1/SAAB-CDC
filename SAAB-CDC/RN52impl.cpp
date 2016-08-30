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
 * Modified by: Karlis Veilands
 * Last modified on: Aug 11, 2016
 */

#include "RN52impl.h"

/**
 * Reads the input (if any) from UART over software serial connection
 */

void RN52impl::readFromUART() {
    while (softSerial.available()) {
        char c = softSerial.read();
        fromUART(c);
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
    } else if (mode == DATA) {
        digitalWrite(BT_CMD_PIN, HIGH);
    }
};

const char *CMD_QUERY = "Q\r";
void RN52impl::onGPIO2() {
    queueCommand(CMD_QUERY);
}

void RN52impl::onProfileChange(BtProfile profile, bool connected) {
    switch(profile) {
        case A2DP:bt_a2dp = connected;
            if (connected && playing) {
                // Serial.println("DEBUG: RN52 connection ok; 'auto-play' should kick in now!");
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
        }
    }
}

/**
 * Initializes Atmel pins and software serial for their initial state on startup
 */

void RN52impl::initialize() {
    softSerial.begin(9600);
    
    // Initializing ATMEGA pins
    pinMode(BT_PWREN_PIN,OUTPUT);
    pinMode(BT_EVENT_INDICATOR_PIN,INPUT);
    pinMode(BT_CMD_PIN, OUTPUT);
    pinMode(BT_FACT_RST_PIN,INPUT);             // Some REALLY crazy stuff is going on if this pin is set as output and pulled low. Leave it alone! Trust me...
    pinMode(HW_REV_CHK_PIN,INPUT);              // HW revision check pin connected to a resistor divider network. We do an analogRead() on this pin and take action accordingly
    pinMode(PIN_A2,OUTPUT);
    pinMode(SN_XCEIVER_RS_PIN,OUTPUT);
    pinMode(PIN_A4,OUTPUT);
    pinMode(PIN_A5,OUTPUT);
    digitalWrite(BT_EVENT_INDICATOR_PIN,HIGH);  // Default state of GPIO2, per data sheet, is HIGH
    digitalWrite(BT_CMD_PIN,HIGH);              // Default state of GPIO9, per data sheet, is HIGH
    
    int hwRevisionCheckValue = analogRead(HW_REV_CHK_PIN);
    
    switch (hwRevisionCheckValue) {
        case (511):                                 // PCBs v3.3A, v4.1 or v4.2 (100K/5K Ohm network); TODO: make sure the correct resistors are soldered on!!!
            time.pulse(BT_PWREN_PIN,3000,0);        // Pulls PWREN pin HIGH for 3000ms, then pulls it LOW thus enabling power to RN52
            break;
        case (91):                                  // PCB v4.3 (100K/10K Ohm network)
            time.pulse(BT_PWREN_PIN,3000,0);        // Pulls PWREN pin HIGH for 3000ms, then pulls it LOW thus enabling power to RN52
            digitalWrite(SN_XCEIVER_RS_PIN,LOW);    // This pin needs to be pulled low, otherwise SN65HVD251D CAN transciever goes into sleep mode
            break;
        default:                                    // PCB revision is older than v3.3A; PWREN is hardwired to 3v3; no other action needs to be taken
            break;
    }
    
    // Configuring RN52
    /*
    Serial.println("Configuring RN52... ");
    set_baudrate();
    waitForResponse();
    delay(1000);
    reboot();
    delay(5000);
    set_discovery_mask();
    waitForResponse();
    delay(1000);
    set_connection_mask();
    waitForResponse();
    delay(1000);
    set_cod();
    waitForResponse();
    delay(1000);
    set_device_name();
    waitForResponse();
    delay(1000);
    set_extended_features();
    waitForResponse();
    delay(1000);
    set_max_volume();
    waitForResponse();
    delay(1000);
    reboot();
     */
}

void RN52impl::waitForResponse() {
    do {
        update();
    } while (currentCommand != NULL);
}