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
    if (argc != 4) { //validate number of args
        perror("ERROR INVALID NUMBER OF INPUTS");
        exit(1);
    }
    int file_path = open(argv[1], O_RDWR); //open message slot device file
    if (file_path < 0) {
        printf("%d", file_path);
        perror("ERROR INVALID FILE PATH");
        exit(1);
    }
    unsigned long channel_id = atoi(argv[2]);
    int set_channel_id = ioctl(file_path, MSG_SLOT_CHANNEL, channel_id); //set channel_id to the specified id
    if (set_channel_id != 0) {
        perror("ERROR IN SETTING FILE CHANNEL ID");
        exit(1);
    }
    char *message = argv[3];
    int message_length = strlen(message);
    int write_message = write(file_path, message, message_length); //write messge to device file
    if (write_message != message_length) {
        perror("ERROR IN WRITE MESSAGE");
        exit(1);
    }
    close(file_path);
    exit(0);
}
