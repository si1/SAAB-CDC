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
 * Modified on: December 9, 2016
 */

#include "CDC.h"
#include "MessageSender.h"
#include "Timer.h"

extern Timer time;

void sendFrame(void *p) {
    Message* msg = (Message*)p;
    CDC.sendCanFrame(msg->frameId, msg->frames[msg->framesSent]);
    msg->framesSent++;
    if (msg->framesSent < msg->frameCount) {
        time.after(msg->interval,sendFrame,msg);
    }
    else {
        msg->frameCount = 0;
    }
}

void MessageSender::sendCanMessage(int frameId, unsigned char frames[][CAN_FRAME_LENGTH], int frameCount, unsigned long interval) {
    for (int i = 0; i < MESSAGE_COUNT; i++) {
        if (messages[i].frameCount == 0) {
            messages[i].frameCount = frameCount;
            messages[i].frameId = frameId;
            messages[i].interval = interval;
            for (int frameNum = 0; frameNum < frameCount; frameNum++) {
                for (int c = 0; c < CAN_FRAME_LENGTH; c++) {
                    messages[i].frames[frameNum][c] = frames[frameNum][c];
                }
            }
            CDC.sendCanFrame(messages[i].frameId, messages[i].frames[0]);
            messages[i].framesSent = 1;
            time.after(messages[i].interval,sendFrame,&messages[i]);
            break;
        }
    }
}
