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

#include "RN52handler.h"

RN52handler BT;

/**
 * Checks for state of event indicator pin (GPIO2). Calls out onGPIO2() from RN52impl that will querry the RN52 module for its status.
 */

void RN52handler::update() {
    driver.readFromUART();
    if (digitalRead(BT_EVENT_INDICATOR_PIN) == 0) {
    if ((millis() - lastEventIndicatorPinStateChange) > 100) {
        lastEventIndicatorPinStateChange = millis();
        driver.onGPIO2();
        }
    }
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

void RN52handler::bt_set_discovery_mask() {
    driver.set_discovery_mask();
}

void RN52handler::bt_set_connection_mask() {
    driver.set_connection_mask();
}

void RN52handler::bt_set_cod() {
    driver.set_cod();
}

void RN52handler::bt_set_device_name() {
    driver.set_device_name();
}

void RN52handler::bt_set_normalized_name() {
    driver.set_normalized_name();
}

void RN52handler::bt_set_maxvol() {
    driver.set_max_volume();
}

void RN52handler::bt_set_extended_features() {
    driver.set_extended_features();
}

void RN52handler::bt_reboot() {
    driver.reboot();
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
                Serial.println("Going _V_isible");
                break;
            case 'I':
                bt_invisible();
                Serial.println("Going _I_nvisible");
                break;
            case 'C':
                bt_reconnect();
                Serial.println("Re_C_onnect");
                break;
            case 'D':
                bt_disconnect();
                Serial.println("_D_isconnect");
                break;
            case 'P':
                bt_play();
                Serial.println("_P_lay");
                break;
            case 'N':
                bt_next();
                Serial.println("_N_ext");
                break;
            case 'R':
                bt_prev();
                Serial.println("P_R_evious");
                break;
            case 'A':
                bt_vassistant();
                Serial.println("_A_ssistant");
                break;
            default:
                break;
        }
    }
}

void RN52handler::initialize() {
    driver.initialize();
}