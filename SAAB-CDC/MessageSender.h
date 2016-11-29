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
