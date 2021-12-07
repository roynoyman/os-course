//
// Created by Roy Noyman on 02/12/2021.
//
#include <printf.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "message_slot.h"


int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "INVALID NUMBER OF INPUTS", strerror(errno));
        exit(1);
    }
    int file_path = open(argv[1], O_WRONLY);
    if (file_path < 0) {
        fprintf(stderr, "INVALID FILE PATH", strerror(errno));
        exit(1);
    }
    unsigned long channel_id = atoi(argv[2]);
    int set_channel_id = ioctl(file_path, MSG_SLOT_CHANNEL, channel_id);
    if (set_channel_id != 0) {
        fprintf(stderr, "ERROR IN SETTING FILE CHANNEL ID", strerror(errno));
        exit(1);
    }
    char *message = argv[3];
    int message_length = strlen(message);
    int write_message = write(file_path, message, message_length);
    close(file_path);
    return 0;
}