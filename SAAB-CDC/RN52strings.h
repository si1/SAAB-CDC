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

#ifndef RN52STRINGS_H_
#define RN52STRINGS_H_

// Action commands
const char *RN52_CMD_BEGIN = "CMD\r\n";
const char *RN52_CMD_EXIT = "END\r\n";
const char *RN52_CMD_QUERY = "Q\r";
const char *RN52_CMD_DETAILS = "D\r";
const char *RN52_CMD_RECONNECTLAST = "B,06\r";
const char *RN52_CMD_DISCONNECT = "K,06\r";
const char *RN52_CMD_REBOOT = "R,1\r";
const char *RN52_CMD_VOLUP = "AV+\r";
const char *RN52_CMD_VOLDOWN = "AV-\r";
const char *RN52_CMD_DISCOVERY_ON = "@,1\r";
const char *RN52_CMD_DISCOVERY_OFF = "@,0\r";

// RN52 settings commands
const char *RN52_SET_DISCOVERY_MASK = "SD,06\r"; // A2DP/AVRCP + SPP profiles
const char *RN52_SET_CONNECTION_MASK = "SK,06\r"; // A2DP/AVRCP + SPP profiles
const char *RN52_SET_COD = "SC,200420\r"; // "Major service class" = Audio; "Major device class" = Audio/Video; "Minor device class" = Car audio
const char *RN52_SET_DEVICE_NAME = "SN,BlueSaab\r"; // Broadcasted and shown in audio source's settigns
const char *RN52_SET_NORMALIZED_NAME = "S-,SAAB\r"; // Not broadcaseted; invisible to end user
const char *RN52_SET_MAXVOL = "SS,0F\r"; // Sets the volume gain to MAX level 15 (default 11)
const char *RN52_SET_EXTENDED_FEATURES = "S%,0084\r"; // Discoverable on startup; Disable system tones


// AVRCP commands
const char *RN52_CMD_AVCRP_NEXT = "AT+\r";
const char *RN52_CMD_AVCRP_PREV = "AT-\r";
const char *RN52_CMD_AVCRP_VASSISTANT = "P\r";
const char *RN52_CMD_AVCRP_PLAYPAUSE = "AP\r";

// RN52 reply messages
const char *RN52_RX_OK = "AOK\r\n";
const char *RN52_RX_ERROR = "ERR\r\n";
const char *RN52_RX_WHAT = "?\r\n";


#endif /* RN52STRINGS_H_ */
