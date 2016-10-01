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
 * Modified on: May 17, 2016
 */

#include <avr/io.h>
#include <avr/wdt.h>
#include "RN52handler.h"

RN52handler BT;

/**
 * Checks for state of event indicator pin (GPIO2). Calls out onGPIO2() from RN52impl that will querry the RN52 module for its status.
 */

void RN52handler::update() {
    driver.update();
}

void RN52handler::bt_play() {
    driver.sendAVCRP(RN52::RN52driver::PLAYPAUSE);
}

void RN52handler::bt_prev() {
    driver.sendAVCRP(RN52::RN52driver::PREV);
}

void RN52handler::bt_next() {
    driver.sendAVCRP(RN52::RN52driver::NEXT);
}

void RN52handler::bt_vassistant() {
    driver.sendAVCRP(RN52::RN52driver::VASSISTANT);
}

void RN52handler::bt_volup() {
    driver.sendAVCRP(RN52::RN52driver::VOLUP);
}

void RN52handler::bt_voldown() {
    driver.sendAVCRP(RN52::RN52driver::VOLDOWN);
}

void RN52handler::bt_visible() {
    driver.visible(true);
}

void RN52handler::bt_invisible() {
    driver.visible(false);
}

void RN52handler::bt_reconnect() {
    driver.reconnectLast();
}

void RN52handler::bt_disconnect() {
    driver.disconnect();
}

void RN52handler::bt_set_maxvol() {
    driver.set_max_volume();
}


/**
 * Debug function used only in 'bench' testing. Listens to input on serial console and calls out corresponding function.
 */

void RN52handler::monitor_serial_input() {
    int incomingByte = 0;
    
    if (Serial.available() > 0) {
        incomingByte = Serial.read();
        switch (incomingByte) {
            case 'V':
                bt_visible();
                Serial.println(F("Going into discoverable mode"));
                break;
            case 'I':
                bt_invisible();
                Serial.println(F("Going into non-discoverable/connectable mode"));
                break;
            case 'C':
                bt_reconnect();
                Serial.println(F("Re-connecting to the last known device"));
                break;
            case 'D':
                bt_disconnect();
                Serial.println(F("Disconnecting from the current device"));
                break;
            case 'P':
                bt_play();
                Serial.println(F("\"Play/Pause\" current track"));
                break;
            case 'N':
                bt_next();
                Serial.println(F("Skip to \"Next\" track"));
                break;
            case 'R':
                bt_prev();
                Serial.println(F("Go back to \"Previous\" track"));
                break;
            case 'A':
                bt_vassistant();
                Serial.println(F("Invoking voice assistant"));
                break;
            case 'B':
                wdt_enable(WDTO_1S);
                Serial.println(F("Rebooting the main uC"));
                break;
            case 'U':
                bt_volup();
                Serial.println(F("Turning RN52 volume up"));
                break;
            case 'O':
                bt_voldown();
                Serial.println(F("Turning RN52 volume down"));
                break;
            case 'M':
                bt_set_maxvol();
                Serial.println(F("Turning RN52 volume to max"));
                break;
            default:
                break;
        }
    }
}

void RN52handler::initialize() {
    driver.initialize();
}