/*************************************************************************
	> File Name: teacher.c
	> Author: suyelu
	> Mail: suyelu@haizeix.com
	> Created Time: 日  3/ 8 05:18:05 2020
 ************************************************************************/

#include "teacher.h"

//char config[50] = "/etc/HelpSys/teacher.conf";
char config[50] = "./teacher.conf";


int main(int argc, char **argv) {
    if (argc != 2) {
        DBG("Useage:dohelp Help-Code\n");
        exit(1);
    }

    int master_port, sockfd, type = 0, code = -1;
    char master_ip[20] = {0}, tmp[20] = {0};

    code = atoi(argv[1]);
    get_conf_value(config, "MasterIp", master_ip);
    get_conf_value(config, "MasterPort", tmp);
    master_port = atoi(tmp);

    if ((sockfd = connect_nonblock(master_port, master_ip, 30000)) < 0) {
        DBG("Connect to Server Error.\n");
        close(sockfd);
        exit(1);
    }

    if (send(sockfd, (void *)&type, sizeof(int), 0) <= 0) {
        DBG("Send User-Type Error.\n");
        close(sockfd);
        exit(1);
    }

    //Send Help-Code to Server
    if (send(sockfd, (void *)&code, sizeof(int), 0) <= 0) {
        DBG("Seng Help-Code Error.\n");
        close(sockfd);
        exit(1);
    } 
    //Recv for Student's Information.
    
    struct Msg_t msg_t;
    
    if (recv(sockfd, (void *)&msg_t, sizeof(msg_t), 0) <= 0) {
        DBG("Recv Student's Information Error.\n");
        close(sockfd);
        exit(1);
    }

    close(sockfd);

    DBG("Connecting to %s's system...\n", msg_t.real_name);

    //Now we have student's information, we need connect to student.
    
    int pid;

    if ((pid = fork()) < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        char user_str[50], cmd_str[100], port_str[20];
        sprintf(user_str, "%s@%s", msg_t.name, master_ip);
        sprintf(cmd_str, "cd %s; bash", msg_t.path);
        sprintf(port_str, "%d", msg_t.port);
        int ret = execl("/usr/bin/ssh", "ssh", "-p", port_str, user_str, "-t", cmd_str, NULL);
        if (ret < 0) {
            perror("execl");
        }
        return 0;
    } else {
        wait(NULL);
    }
    return 0;
}