/*
 * C++ Class for handling sending of groups of CAN frames in specified intervals on SAAB I-Bus
 * Copyright (C) 2016 Girts Linde
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
 * Created by: Girts Linde
 * Created on: November 28, 2016
 * Modified by: Girts Linde
 * Modified on: November 28, 2016
 */

#ifndef MESSAGESENDER_H
#define MESSAGESENDER_H

const int CAN_FRAME_LENGTH = 8;
const int MESSAGE_COUNT = 3;

struct Message {
    int frameId;
    unsigned char frames[4][CAN_FRAME_LENGTH];
    int frameCount;
    int framesSent;
    unsigned long interval;
};

class MessageSender {
    Message messages[MESSAGE_COUNT];
public:
    MessageSender() {
        for (int i = 0; i < MESSAGE_COUNT; i++) {
            messages[i].frameCount = 0;
        }
    }
    void sendCanMessage(int frameId, unsigned char frames[][CAN_FRAME_LENGTH], int frameCount, unsigned long interval);
};


#endif
