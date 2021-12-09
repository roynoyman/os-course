#include "message_slot.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 3) { //validate number of args
        perror("ERROR INVALID NUMBER OF INPUTS");
        exit(1);
    }
    int file_path = open(argv[1], O_RDONLY); //open message slot device file
    if (file_path < 0) {
        perror("ERROR INVALID FILE PATH");
        exit(1);
    }
    unsigned int channel_id = atoi(argv[2]);
    int set_channel_id = ioctl(file_path, MSG_SLOT_CHANNEL, channel_id); //set channel_id to the specified id
    if (set_channel_id != 0) {
        perror("ERROR IN SETTING FILE CHANNEL ID");
        exit(1);
    }
    char message_buffer[BUFFER_LENGTH];
    int message = read(file_path, message_buffer, BUFFER_LENGTH); //read the message from the slot to the buffer
    if (message < 0) {
        perror("ERROR WHILE READING MESSAGE");
        exit(1);
    }
    if (write(STDOUT_FILENO, message_buffer, message) < 0) { //print the message to st output
        perror("ERROR WHILE WRITING MESSAGE");
        exit(1);
    }
    close(file_path);
    return 0;
}
