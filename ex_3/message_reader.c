//
// Created by Roy Noyman on 02/12/2021.
//
#include "message_slot.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "INVALID NUMBER OF INPUTS", strerror(errno));
        exit(1);
    }
    int file_path = open(argv[1], O_RDONLY);
    if (file_path < 0) {
        fprintf(stderr, "INVALID FILE PATH", strerror(errno));
        exit(1);
    }
    unsigned int channel_id = atoi(argv[2]);
    int set_channel_id = ioctl(file_path, MSG_SLOT_CHANNEL, channel_id);
    if (set_channel_id != 0) {
        fprintf(stderr, "ERROR IN SETTING FILE CHANNEL ID", strerror(errno));
        exit(1);
    }
    char message_buffer[BUFFER_LENGTH];
    int message = read(file_path, message_buffer, BUFFER_LENGTH);
    if (message < 0) {
        fprintf(stderr, "ERROR WHILE READING MESSAGE", strerror(errno));
        exit(1);
    }
    if (write(STDOUT_FILENO, message_buffer, message) < 0){
        fprintf(stderr, "ERROR WHILE WRITING MESSAGE", strerror(errno));
        exit(1);
    }
    close(file_path);
    return 0;
}