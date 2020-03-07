#ifndef COMMON_H
#define COMMON_H


#include "head.h"


#define MAX_SIZE 1024
#define MASTER_PORT 8731

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

int socket_accept(int sock_listen);

int socket_connect(int port, char *host);

int connect_nonblock(int port, char *host, long timeout);

int connect_sock(struct sockaddr_in addr);

bool check_connect(struct sockaddr_in addr, long timeout);

int recv_data(int sockfd, char* buf, int bufsize);

int send_response(int sockfd, int req); //

int recv_response(int sockfd); //

int generate_logname(int code, char *logname, char *logdir);

int check_size(char *filename, int size, char *dir);

//文件大于size M 的时候做压缩备份

int write_Pi_log (char *PiHealthLog, const char *format, ...);



#endif
