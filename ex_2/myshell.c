#include <printf.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

//
// Created by Roy Noyman on 11/11/2021.
//
int prepare(void) {
    printf("Prepare your sheel");
    // What do we want to check before? To be completed/removed
    return 0;
}

int contains_special_character_at_index(int count, char **arglist) {
    int i = 0;
    for (i = 0; i < count - 1; i++) {
        if (strcmp(arglist[i], '|')) {
            arglist[i] = NULL;
            return i;
        } else if (strcmp(arglist[i], '>')) {
            return i;
        }
    }
    if (strcmp(arglist[count - 1], '&')) {
        return count - 1;
    }
    return 0;
}

int process_arglist(int count, char **arglist) {
    int special_character_index = contains_special_character_at_index(count, arglist);
    if (special_character_index == 0) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("ERROR: FORK FAILURE");
            exit(1);
        }
        if (pid == 0) {
            if(execvp(arglist[0], arglist)==-1){
                exit(errno);
            }
        }
        else{
            waitpid(pid, )
        }
    }
    char special_char = arglist[special_character_index];
    if (special_char == 'NULL') { //means pipe

    } else if (special_char == '&') {

    } else { //means >

    }
    return 0;
}

int main(void) {
    char *check[] = {{"abc", "|", "bla"},
                     {"cde", "gfgf"}};
    printf("%s", check[1]);
    if (strchr(check[1], '|')) {
        printf("bla");
    }
}