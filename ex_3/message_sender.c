//
// Created by Roy Noyman on 02/12/2021.
//
#include <linux/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "message_slot.h"


int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "INVALID NUMBER OF INPUTS");
        exit(1);
    }
    printf("%s\n",argv[1]);
    int file_path = open(argv[1], O_RDWR);
    if (file_path < 0) {
        printf("%d",file_path);
        fprintf(stderr, "INVALID FILE PATH");
        exit(1);
    }
    unsigned long channel_id = atoi(argv[2]);
    int set_channel_id = ioctl(file_path, MSG_SLOT_CHANNEL, channel_id);
    if (set_channel_id != 0) {
        fprintf(stderr, "ERROR IN SETTING FILE CHANNEL ID");
        exit(1);
    }
    char *message = argv[3];
    int message_length = strlen(message);
    int write_message = write(file_path, message, message_length);
    if(write_message != message_length){
        fprintf(stderr, "ERROR IN WRITE MESSAGE");
        exit(1);
    }
    close(file_path);
    return 0;
}