/*************************************************************************
	> File Name: test.c
	> Author: suyelu
	> Mail: suyelu@haizeix.com
	> Created Time: 三 10/21 21:39:13 2020
 ************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
int main() {
    int pid;
    if ((pid = fork()) < 0) {
        perror("fork()");
        exit(1);
    }
    if (pid == 0) {
        execlp("tmux", "tmux", "new-session", "-s", "name", NULL);
    } else {
        wait(NULL);
    }
    return 0;
}
