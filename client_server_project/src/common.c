#include "common.h"

void zombie_removal(){
    int status;
    pid_t pid;
    // recycling all the child processes
    while ((pid = waitpid(-1, &status, 0)) > 0){
    }
    if (errno != ECHILD) {
        perror("Waitpid error: still have some zombie processes");
    }
}