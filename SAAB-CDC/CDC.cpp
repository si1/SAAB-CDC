/*
 * C++ Class for handling CD changer emulator on SAAB I-Bus
 * Copyright (C) 2016  Karlis Veilands
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
 * Created by: Karlis Veilands
 * Created on: Jun 4, 2015
 * Modified by: Karlis Veilands
 * Modified on: May 17, 2016
 */

#include <Arduino.h>
#include "CAN.h"
#include "CDC.h"
#include "RN52handler.h"
#include "Timer.h"

/**
 * Variables:
 */

extern Timer time;
void sendCdcNodeStatus(void*);
void sendCdcActiveStatus(void*);
void sendCdcPowerdownStatus(void*);
void *currentCdcCmd = NULL;
unsigned long cdcStatusLastSendTime = 0;            // Timer used to ensure we send the CDC status frame in a timely manner
unsigned long lastIcomingEventTime = 0;             // Timer used for determening if we should treat current event as, for example, a long press of a button
boolean sidTextControlTestMode = false;             // True while SID text control on behalf of SPA is enabled. Toggled ON/OFF with button #5 on IHU
boolean cdcActive = false;                          // True while our module, the simulated CDC, is active
boolean cdcStatusResendNeeded = false;              // True if something has triggered the need to send the CDC status frame as an event
boolean cdcStatusResendDueToCdcCommand = false;     // True if the need for sending the CDC status frame was triggered by a CDC command
boolean writeTextOnDisplayTimerActive = false;      // True while we are writing custom text on SID every SID_CONTROL_TX_BASETIME interval
int incomingEventCounter = 0;                       // Counter for incoming events to determine when we will treat the event, for example, as a long press of a button
int displayRequestTimerId = -1;
int writeTextOnDisplayTimerId = -1;
int currentTimerEvent = -1;
int cdcPoweronCmd[NODE_STATUS_TX_MSG_SIZE][9] = {
    {0x32,0x00,0x00,0x03,0x01,0x02,0x00,0x00,-1},
    {0x42,0x00,0x00,0x22,0x00,0x00,0x00,0x00,-1},
    {0x52,0x00,0x00,0x22,0x00,0x00,0x00,0x00,-1},
    {0x62,0x00,0x00,0x22,0x00,0x00,0x00,0x00,-1}
};
int cdcActiveCmd[NODE_STATUS_TX_MSG_SIZE] [9] = {
    {0x32,0x00,0x00,0x16,0x01,0x02,0x00,0x00,-1},
    {0x42,0x00,0x00,0x36,0x00,0x00,0x00,0x00,-1},
    {0x52,0x00,0x00,0x36,0x00,0x00,0x00,0x00,-1},
    {0x62,0x00,0x00,0x36,0x00,0x00,0x00,0x00,-1},
};
int cdcPowerdownCmd[NODE_STATUS_TX_MSG_SIZE] [9] = {
    {0x32,0x00,0x00,0x19,0x01,0x00,0x00,0x00,-1},
    {0x42,0x00,0x00,0x38,0x01,0x00,0x00,0x00,-1},
    {0x52,0x00,0x00,0x38,0x01,0x00,0x00,0x00,-1},
    {0x62,0x00,0x00,0x38,0x01,0x00,0x00,0x00,-1}
};

/* Format of SOUND_REQUEST frame:
 ID: SOUND_REQUEST
 [0]: Sent on basetime/event; 0 = Basetime; 80 = Event
 [1]: Type of sound
 [2-7]: Zeroed out; not in use
 */
int soundCmd[] = {0x80,SOUND_ACK,0x00,0x00,0x00,0x00,0x00,0x00,-1};

/* Format of GENERAL_STATUS_CDC frame:
 ID: CDC node ID
 [0]:
    byte 0, bit 7: FCI NEW DATA: 0 - sent on basetime, 1 - sent on event
    byte 0, bit 6: FCI REMOTE CMD: 0 - status change due to internal operation, 1 - status change due to CDC_COMMAND frame
    byte 0, bit 5: FCI DISC PRESENCE VALID: 0 - disc presence signal is not valid, 1 - disc presence signal is valid
 [1]: Disc presence validation (boolean)
    byte 1-2, bits 0-15: DISC PRESENCE: (bitmap) 0 - disc absent, 1 - disc present. Bit 0 is disc 1, bit 1 is disc 2, etc.
 [2]: Disc presence (bitmap)
    byte 1-2, bits 0-15: DISC PRESENCE: (bitmap) 0 - disc absent, 1 - disc present. Bit 0 is disc 1, bit 1 is disc 2, etc.
 [3]: Disc number currently playing
    byte 3, bits 7-4: DISC MODE
    byte 3, bits 3-0: DISC NUMBER
 [4]: Track number currently playing
 [5]: Minute of the current track
 [6]: Second of the current track
 [7]: CD changer status; D0 = Married to the car
 */
int cdcGeneralStatusCmd[] = {0xE0,0xFF,0x3F,0x41,0xFF,0xFF,0xFF,0xD0,-1};

/* Format of DISPLAY_RESOURCE_REQ frame:
 ID: Node ID requesting to write on SID
 [0]: Request source
 [1]: SID object to write on; 0 = entire SID; 1 = 1st row; 2 = 2nd row
 [2]: Request type: 1 = Engineering test; 2 = Emergency; 3 = Driver action; 4 = ECU action; 5 = Static text; 6 = We don't want to write on SID
 [3]: Request source function ID
 [4-7]: Zeroed out; not in use
 */
int displayRequestCmd[] = {SPA_APL_ADR,0x02,0x05,SPA_SID_FUNCTION_ID,0x00,0x00,0x00,0x00,-1}; // We pretend to be SPA and want a write access to 2nd row of SID

/**
 * DEBUG: Prints the CAN TX frame to serial output
 */

void CDChandler::printCanTxFrame() {
    Serial.print(CAN_TxMsg.id,HEX);
    Serial.print(" Tx-> ");
    for (int i = 0; i < 8; i++) {
        Serial.print(CAN_TxMsg.data[i],HEX);
        Serial.print(" ");
    }
    Serial.println();
}

/**
 * DEBUG: Prints the CAN RX frame to serial output
 */

void CDChandler::printCanRxFrame() {
    Serial.print(CAN_RxMsg.id,HEX);
    Serial.print(" Rx-> ");
    for (int i = 0; i < 8; i++) {
        Serial.print(CAN_RxMsg.data[i],HEX);
        Serial.print(" ");
    }
    Serial.println();
}

/**
 * Opens CAN bus for communication
 */

void CDChandler::openCanBus() {
    CAN.begin(47);                // SAAB I-Bus is 47.619kbps
    CAN_TxMsg.header.rtr = 0;     // This value never changes
    CAN_TxMsg.header.length = 8;  // This value never changes
}

/**
 * Handles an incoming (RX) frame
 */

void CDChandler::handleRxFrame() {
    if (CAN.CheckNew()) {
        CAN_TxMsg.data[0]++;
        CAN.ReadFromDevice(&CAN_RxMsg);
        switch (CAN_RxMsg.id) {
            case NODE_STATUS_RX_IHU:
                /*
                 Here be dragons... This part of the code is responsible for causing lots of headache
                 We look at the bottom half of 3rd byte of '6A1' frame to determine what 'current_cdc_command' should be
                 */
                switch (CAN_RxMsg.data[3] & 0x0F){
                    case (0x3):
                        currentCdcCmd = cdcPoweronCmd;
                        sendCdcNodeStatus(NULL);
                        break;
                    case (0x2):
                        currentCdcCmd = cdcActiveCmd;
                        sendCdcNodeStatus(NULL);
                        break;
                    case (0x8):
                        currentCdcCmd = cdcPowerdownCmd;
                        sendCdcNodeStatus(NULL);
                        break;
                }
                break;
            case CDC_CONTROL:
                handleIhuButtons();
                break;
            case STEERING_WHEEL_BUTTONS:
                handleSteeringWheelButtons();
                break;
            case DISPLAY_RESOURCE_GRANT:
                if (CAN_RxMsg.data[0] == SID_OBJECT2) {
                    if (CAN_RxMsg.data[1] == SPA_SID_FUNCTION_ID) {
                        // Serial.println("DEBUG: We have been granted the right to write text to the second row in the SID");
                        if (!writeTextOnDisplayTimerActive) {
                            writeTextOnDisplayTimerId = time.every(SID_CONTROL_TX_BASETIME, &writeTextOnDisplayOnTime,NULL);
                            writeTextOnDisplayTimerActive = true;
                        }
                    }
                    else {
                        // ”OK To Write” = false
                    }
                }
                else if ((CAN_RxMsg.data[0] == 0x00) && (CAN_RxMsg.data[1] != 0xFF) && (SID_OBJECT2 != 0))  {
                    // ”OK To Write” = false
                }
                else if ((CAN_RxMsg.data[0] != 0x00) && (CAN_RxMsg.data[1] != 0xFF) && (SID_OBJECT2 == 0))  {
                    // ”OK To Write” = false
                }
                else {
                    // No action is taken; The status of ”OK To Write” is not changed
                }
                break;
        }
    }
}

/**
 * Handles the CDC_CONTROL frame that the IHU sends us when it wants to control some feature of the CDC
 */

void CDChandler::handleIhuButtons() {
    boolean event = (CAN_RxMsg.data[0] == 0x80);
    if (!event) {
        /*
         TODO: Pay attention at what's happening in this case
         Occasionally there's a frame: 0x00 0x24 0x00 0x00 0x00 0x00 0x00 0x00; can we ignore this just because it wasn't sent on an event?
        */
        return;
    }
    switch (CAN_RxMsg.data[1]) {
        case 0x24: // CDC = ON (CD/RDM button has been pressed twice)
            BT.bt_reconnect();
            cdcActive = true;
            //displayRequestTimerId = time.every(SID_CONTROL_TX_BASETIME, &sendDisplayRequestOnTime,NULL);
            sendCanFrame(SOUND_REQUEST, soundCmd);
            break;
        case 0x14: // CDC = OFF (Back to Radio or Tape mode)
            //time.stop(displayRequestTimerId);
            //time.stop(writeTextOnDisplayTimerId);
            BT.bt_disconnect();
            cdcActive = false;
            writeTextOnDisplayTimerActive = false;
            break;
        default:
            break;
    }
    if (cdcActive) {
        checkCanEvent(1);
        switch (CAN_RxMsg.data[1]) {
            case 0x59: // NXT
                BT.bt_play();
                break;
            case 0x84: // SEEK button (middle) long press on IHU
                BT.bt_visible();
                break;
            case 0x88: // > 2 sec long press of SEEK button (middle) on IHU
                BT.bt_invisible();
                break;
            case 0x76: // Random ON/OFF (Long press of CD/RDM button)
                break;
            case 0xB1: // Pause ON
                // N/A for now
                break;
            case 0xB0: // Pause OFF
                // N/A for now
                break;
            case 0x35: // Track +
                BT.bt_next();
                break;
            case 0x36: // Track -
                BT.bt_prev();
                break;
            case 0x68: // IHU buttons "1-6"
                switch (CAN_RxMsg.data[2]) {
                    case 0x01: // Button "1" on IHU
                        BT.bt_volup();
                        break;
                    case 0x02: // Button "2" on IHU
                        BT.bt_set_maxvol();
                        break;
                    case 0x03: // Button... aren't we f***ing smart? Take it from here :)
                        BT.bt_reconnect();
                        break;
                    case 0x04:
                        BT.bt_voldown();
                        break;
                    case 0x05:
                        /*
                        if (!sidTextControlTestMode) {
                            displayRequestTimerId = time.every(SID_CONTROL_TX_BASETIME, &sendDisplayRequestOnTime,NULL);
                            sidTextControlTestMode = true;
                            sendCanFrame(SOUND_REQUEST, soundCmd);
                        }
                        else {
                            time.stop(displayRequestTimerId);
                            time.stop(writeTextOnDisplayTimerId);
                            sidTextControlTestMode = false;
                            writeTextOnDisplayTimerActive = false;
                            sendCanFrame(SOUND_REQUEST, soundCmd);
                        }
                        */
                        break;
                    case 0x06:
                        BT.bt_disconnect();
                    default:
                        break;
                }
            default:
                break;
        }
    }
}

/**
 * Handles the STEERING_WHEEL_BUTTONS frame
 * TODO connect the SID button events to actions
 */

void CDChandler::handleSteeringWheelButtons() {
    if (cdcActive) {
        checkCanEvent(4);
        switch (CAN_RxMsg.data[2]) {
            case 0x04: // NXT button on wheel
                //BT.bt_play();
                break;
            case 0x10: // Seek+ button on wheel
                //BT.bt_next();
                break;
            case 0x08: // Seek- button on wheel
                //BT.bt_prev();
                break;
            default:
                //Serial.print(CAN_RxMsg.data[2],HEX);
                //Serial.println("DEBUG: Unknown button message");
                break;
        }
    }
}

/**
 * Handles CDC status and sends it to IHU as necessary
 */

void CDChandler::handleCdcStatus() {
    
    handleRxFrame();
    
    // If the CDC status frame needs to be sent as an event, do so now
    // (note though, that we may not send the frame more often than once every 50 ms)
    
    if (cdcStatusResendNeeded && (millis() - cdcStatusLastSendTime > 100)) {
        sendCdcStatus(true, cdcStatusResendDueToCdcCommand);
    }
}

void CDChandler::sendCdcStatus(boolean event, boolean remote) {

    sendCanFrame(GENERAL_STATUS_CDC, cdcGeneralStatusCmd);

    // Record the time of sending and reset status variables
    cdcStatusLastSendTime = millis();
    cdcStatusResendNeeded = false;
    cdcStatusResendDueToCdcCommand = false;
    
}

/**
 * Sends a request for using the SID, row 2. We may NOT start writing until we've received a grant frame with the correct function ID!
 */

void CDChandler::sendDisplayRequest() {
    sendCanFrame(DISPLAY_RESOURCE_REQ, displayRequestCmd);
}

/**
 * Formats and puts a frame on CAN bus
 */

void CDChandler::sendCanFrame(int messageId, int *msg) {
    CAN_TxMsg.id = messageId;
    int i = 0;
    while (msg[i] != -1) {
        CAN_TxMsg.data[i] = msg[i];
        i++;
    }
    CAN.send(&CAN_TxMsg);
}

/**
 * Sends a reply of four messages to '6A1' requests
 */

void sendCdcNodeStatus(void *p) {
    int i = (int)p;
    
    if (currentTimerEvent > NODE_STATUS_TX_MSG_SIZE) {
        time.stop(currentTimerEvent);
    }
    CDC.sendCanFrame(NODE_STATUS_TX_CDC, ((int(*)[9])currentCdcCmd)[i]);
    if (i < NODE_STATUS_TX_MSG_SIZE) {
        currentTimerEvent = time.after(NODE_STATUS_TX_INTERVAL,sendCdcNodeStatus,(void*)(i + 1));
    }
    
    else currentTimerEvent = -1;
}

/**
 * Sends CDC status every CDC_STATUS_TX_BASETIME interval
 */

void sendCdcStatusOnTime(void*) {
    CDC.sendCdcStatus(false, false);
}

/**
 * Sends display request every SID_CONTROL_TX_BASETIME interval
 */

void sendDisplayRequestOnTime(void*) {
    CDC.sendDisplayRequest();
}

/**
 * Writes provided text every SID_CONTROL_TX_BASETIME interval
 */

void writeTextOnDisplayOnTime(void*) {
    CDC.writeTextOnDisplay(MODULE_NAME);
}

/**
 * Formats provided text for writing on the SID. This function assumes that we have been granted write access. Do not call it if we haven't!
 * NOTE the character set used by the SID is slightly nonstandard. "Normal" characters should work fine.
 */

void CDChandler::writeTextOnDisplay(char text[]) {

    if (!text) {
        return;
    }
    // Copy the provided string and make sure we have a new array of the correct length
    char txt[15];
    int i, n;
    n = strlen(text);
    n = n > 12 ? 12 : n;
    for (i = 0; i < n; i++) {
        txt[i] = text[i];
    }
    for (i = n + 1; i < 16; i++) {
        txt[i] = 0;
    }

    CAN_TxMsg.id = WRITE_TEXT_ON_DISPLAY;
     
    CAN_TxMsg.data[0] = 0x42; // TODO: check if this is really correct? According to the spec, the 4 shouldn't be there? It's just a normal transport layer sequence numbering?
    CAN_TxMsg.data[1] = 0x96; // Address of the SID
    CAN_TxMsg.data[2] = 0x02; // Sent on basetime; writing to row 2
    CAN_TxMsg.data[3] = txt[0];
    CAN_TxMsg.data[4] = txt[1];
    CAN_TxMsg.data[5] = txt[2];
    CAN_TxMsg.data[6] = txt[3];
    CAN_TxMsg.data[7] = txt[4];
    CAN.send(&CAN_TxMsg);
    
    CAN_TxMsg.data[0] = 0x01; // message 1
    CAN_TxMsg.data[1] = 0x96; // Address of the SID
    CAN_TxMsg.data[2] = 0x02; // Sent on basetime; writing to row 2
    CAN_TxMsg.data[3] = txt[5];
    CAN_TxMsg.data[4] = txt[6];
    CAN_TxMsg.data[5] = txt[7];
    CAN_TxMsg.data[6] = txt[8];
    CAN_TxMsg.data[7] = txt[9];
    CAN.send(&CAN_TxMsg);
    
    CAN_TxMsg.data[0] = 0x00; // message 0
    CAN_TxMsg.data[1] = 0x96; // Address of the SID
    CAN_TxMsg.data[2] = 0x02; // Sent on basetime; writing to row 2
    CAN_TxMsg.data[3] = txt[10];
    CAN_TxMsg.data[4] = txt[11];
    CAN_TxMsg.data[5] = txt[12];
    CAN_TxMsg.data[6] = txt[13];
    CAN_TxMsg.data[7] = txt[14];
    CAN.send(&CAN_TxMsg);
}

/**
 * Checks for a long press of a button event
 * A long press is considered if the first byte of CAN frame != 0x80
 * Then we look at 'frameElement' passed in by function call to look at which element we should put the counter on
 * Once we reach 3 on the counter, we assert that a certain button has been held for a while and take an action accordingly
 * LAST_EVENT_IN_TIMEOUT indicates how many milliseconds have to pass till we reset all the counters and wait for the next potential long press to come in
 */

void CDChandler::checkCanEvent(int frameElement) {
    boolean event = (CAN_RxMsg.data[0] == 0x80);
    if (!event && (CAN_RxMsg.data[frameElement]) != 0) { // Long press of a steering wheel button has taken place.
        if (millis() - lastIcomingEventTime > LAST_EVENT_IN_TIMEOUT) {
            incomingEventCounter = 0;
        }
        incomingEventCounter++;
        lastIcomingEventTime = millis();
        if (incomingEventCounter == 3) {
            switch (CAN_RxMsg.data[frameElement]) {
                case 0x04: // Long press of NXT button on steering wheel
                    BT.bt_vassistant();
                    // Serial.println("NXT long press on steering wheel");
                    break;
                case 0x45: // SEEK+ button long press on IHU
                    BT.bt_visible();
                    // Serial.println("SEEK+ long press on IHU");
                    break;
                case 0x46: // SEEK- button long press on IHU
                    BT.bt_invisible();
                    // Serial.println("SEEK- long press on IHU");
                    break;
                default:
                    break;
            }
        }
    }
    return;
}
