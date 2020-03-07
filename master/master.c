/*************************************************************************
	> File Name: master.c
	> Author: suyelu
	> Mail: suyelu@haizeix.com
	> Created Time: 六  3/ 7 21:23:05 2020
 ************************************************************************/

#include "master.h"

//char config[50] = "/etc/HelpSys/master.conf";
char config[50] = "./master.conf";
char tmp[20] = {0};
struct Stu student[MAX];
pthread_t client_t[MAX];
int client_fd[MAX], sub_index[MAX];
int size, sum = 0;

bool check_online(char *username) {
    for(int i = 0; i < size; i++) {
        if (strcmp(username, student[i].name) == 0) {
            return true;
        }
    }
    return false;
}


void *work(void *arg) {
    int ind = *(int*)arg;
    DBG("%d\n", ind);
    struct Msg first_msg;
    if (recv(client_fd[ind], (void *)&first_msg, sizeof(first_msg), 0) <= 0) {
        perror("recv");
        student[ind].flag = false;
        close(client_fd[ind]);
        return NULL;
    }
    if (check_online(first_msg.name)) {
       //名字重复
        student[ind].flag = false;
        close(client_fd[ind]);
        return NULL;
    }

    if (recv(client_fd[ind], (void *)&first_msg, sizeof(first_msg), 0) <= 0) {
        DBG("Client closed.\n");
        close(client_fd[ind]);
        student[ind].flag = false;
    }
    return NULL;
}

int main() {
    int  master_port, start_port, master_listen, client_in;
    get_conf_value(config, "ConSize", tmp);
    size = atoi(tmp);
    get_conf_value(config, "MasterPort", tmp);
    master_port = atoi(tmp);
    get_conf_value(config, "StartPort", tmp);
    start_port = atoi(tmp);
    DBG("Config done.\n");
    memset(student, 0, sizeof(struct Stu) * MAX);
    DBG("Mem clean.\n");

    master_listen = socket_create(master_port);

    while (1) {
        DBG("Master listening.\n");
        if ((client_in = accept(master_listen, NULL, NULL)) < 0) {
            perror("accept()");
            exit(1);
        }
        int sub = -1;
        for (int i = 0; i < size; i++) {
            if (student[i].flag == false) {
                sub = i;
                break;
            }
        }
        sum++;
        if (sub < 0) {
            //服务端端口已全部用完
            //通知客户端无法建立连接，断开连接，并进入下次循环
        }
        student[sub].flag = true;
        client_fd[sub] = client_in;
        sub_index[sub] = sub;
        pthread_create(&client_t[sub], NULL, work, (void *)&sub_index[sub]);
    }

    return 0;
}
