//
// Created by Roy Noyman on 02/12/2021.
//

#ifndef UNTITLED_MESSAGE_SLOT_H
#define UNTITLED_MESSAGE_SLOT_H

#include <linux/ioctl.h>
#define MAJOR_NUMBER 240
#define BUFFER_LENGTH 128
#define MAX_MINORS 256
#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUMBER, 0, unsigned int)
#define MY_CHAR_DEV "MY_CHAR_DEV"

#endif //UNTITLED_MESSAGE_SLOT_H