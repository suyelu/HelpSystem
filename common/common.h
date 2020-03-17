#ifndef COMMON_H
#define COMMON_H
#define _DEBUG

#include "head.h"
#define VER "2.0"

#define MAX_SIZE 1024

struct Msg {
    char name[20];
    char real_name[20];
    char path[50];
};

struct Msg_t {
    char name[20];
    char real_name[20];
    char path[50];
    int port;
};

struct Code {
    int code;
    int port;
};


int get_conf_value(char *pathname, const char* key_name, char *value);

int socket_create(int port);

int connect_nonblock(int port, char *host, long timeout);

#endif
