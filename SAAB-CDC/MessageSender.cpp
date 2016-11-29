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
