#include <printf.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>

//
// Created by Roy Noyman on 11/11/2021.
//
// reference - rec3
int check_wait_status(pid_t pid) {
    int status, wait_status;
    wait_status = waitpid(pid, &status, WUNTRACED); //in order to stop waiting WUNTRACED collect the status of child.
    printf("%d\n",wait_status);
    if (wait_status < 0) {
        fprintf(stderr, "ERROR: WAIT FAILURE: %s", strerror(errno));
        return 0;
    }
    return wait_status;
}

void terminate_signal_handler() {
    printf("handling termination signal");
}

void child_signal_handler() {
    printf("handling child signal\n");
}


void register_signal_handling(int signum) {
    struct sigaction new_action;
    memset(&new_action, 0, sizeof(new_action));
    if (signum == SIGINT) {
        new_action.sa_sigaction = terminate_signal_handler;
        new_action.sa_flags = SA_RESTART;
    } else if (signum == SIGCHLD) {
        new_action.sa_sigaction = child_signal_handler;
        new_action.sa_flags = SA_NOCLDWAIT;
    } else if (signum == 5) { //reset to default for pipe and redirect
        new_action.sa_handler = SIG_DFL;
        new_action.sa_flags = SA_RESTART;
    }
    if (sigaction(signum, &new_action, NULL) == -1) {
        perror("ERROR: SIGNAL HANDLER FAILURE ");
        exit(1);
    }
}

void check_fork(pid_t pid) {
    if (pid < 0) {
        perror("ERROR: FORK FAILURE");
        exit(0);
    }
}

int prepare(void) {
//    register_signal_handling(SIGINT);
    printf("do prepare\n");
//    register_signal_handling(SIGCHLD);
    return 0;
}

int contains_special_character_at_index(int count, char **arglist) {
    int i;
    for (i = 0; i < count - 1; i++) {
        if (strcmp(arglist[i], "|")) {
            arglist[i] = NULL;
            return i;
        } else if (strcmp(arglist[i], ">")) {
            return i;
        }
    }
    if (strcmp(arglist[count - 1], "&")) {
        arglist[count - 1] = NULL;
        return count - 1;
    }
    return 0;
}

int exec_with_pipe(char **arglist, int index) {
    char **arglist_part_a = arglist;
    char **arglist_part_b = arglist + index + 1;
    int pipefds[2];
    int wait_status_1, wait_status_2;
    if (pipe(pipefds) == -1) {
        fprintf(stderr, "ERROR: PIPE FAILURE: %s", strerror(errno));
        return 0;
    }
    int readerfd = pipefds[0];
    int writerfd = pipefds[1];
    pid_t pid_1 = fork();
    check_fork(pid_1);
    pid_t pid_2 = fork();
    check_fork(pid_2);
    if (pid_1 == 0) {
        register_signal_handling(5);
        close(readerfd);
        if ((dup2(writerfd, STDOUT_FILENO) == -1) || (errno == EINTR)) {
            fprintf(stderr, "ERROR: DUP2 OF PID_1 FAILURE: %s", strerror(errno));
            exit(1);
        }
        close(writerfd);
        execvp(arglist_part_a[0], arglist_part_a);
        perror(arglist_part_a[0]);
        exit(1);
    } else if (pid_2 == 0) {
        register_signal_handling(5);
        close(writerfd);
        if ((dup2(readerfd, STDIN_FILENO) == -1) || (errno == EINTR)) {
            fprintf(stderr, "ERROR: DUP2 OF PID_2: %s", strerror(errno));
            exit(1);
        }
        close(readerfd);
        execvp(arglist_part_b[0], arglist_part_b);
        perror(arglist[0]);
        exit(1);
    } else {
        close(readerfd);
        close(writerfd);
        wait_status_1 = check_wait_status(pid_1);
        wait_status_2 = check_wait_status(pid_2);
        if ((wait_status_1 == 0) || (wait_status_2 == 0)) {
            return 0;
        }
        return 1;
    }
}

int exec_with_redirecting(char **arglist, int index) {
    int fd = open(arglist[index - 1], O_WRONLY | O_CREAT, O_APPEND);
    arglist[index - 2] = NULL;
    pid_t pid = fork();
    int wait_status;
    check_fork(pid);
    if (pid == 0) { //child
        if ((dup2(STDOUT_FILENO, fd) == -1) || (errno == EINTR)) {
            fprintf(stderr, "ERROR: DUP2 OF FD: %s", strerror(errno));
            exit(1);
        }
        execvp(arglist[0], arglist);
        close(fd);
        perror(arglist[0]);
        exit(1);
    } else {
        wait_status = check_wait_status(pid);
        if (wait_status == 0) {
            return 0;
        }
        close(fd);
        return 1;
    }
}


int process_arglist(int count, char **arglist) {
    int special_character_index = contains_special_character_at_index(count, arglist);
    int wait_status;
    int i;
    char **bla = arglist;
    printf("%d\n",count);
    for (i=0; i<count; i++){
        printf("%s\n",bla[i]);
    }
    if (special_character_index == 0) { //no special character
        pid_t pid = fork();
//        check_fork(pid);
        if (pid == 0) {
            printf("im son proccess\n");
            printf("%s%d%s%d\n","pid: ",getpid()," ppid: ",getppid());
//            register_signal_handling(5);
            if (execvp(bla[1], bla) == -1) {
                printf("execvp problem\n");
                fprintf(stderr, "ERROR: EXECVP FAILURE: %s", strerror(errno));
                return 0;
            }
            else{
                printf("did execvp \n");
            }
        } else {
            waitpid(pid,NULL,WUNTRACED);
            printf("done waiting\n");
//            wait_status = check_wait_status(pid);
//            if (wait_status == 0) {
//                return 0;
//            }
            return 1;
        }
    }
    char special_char = *arglist[special_character_index];
    if (special_character_index == count - 1) { // means '&'
        pid_t pid = fork();
        check_fork(pid);
        if (pid == 0) {
            if (execvp(arglist[0], arglist) == -1) {
                fprintf(stderr, "ERROR: EXECVP FAILURE: %s", strerror(errno));
                return 0;
            }
        }
        return 1;
    } else if (special_char == '\0') { //means '|'
        return exec_with_pipe(arglist, special_character_index);
    } else { //means >
        return exec_with_redirecting(arglist, count);
    }
}

int finalize() {
    return 0;
}