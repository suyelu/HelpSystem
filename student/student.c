/*************************************************************************
	> File Name: student.c
	> Author: suyelu
	> Mail: suyelu@haizeix.com
	> Created Time: 日  3/ 8 00:47:57 2020
 ************************************************************************/

#include "student.h"
#include <pwd.h>

//char config[50] = "/etc/HelpSys/student.conf";
char config[50] = "./student.conf";


int main() {
    int master_port, sockfd;
    char master_ip[20] = {0}, real_name[20] = {0}, name[20] = {0};
    char tmp[20] = {0};
    
    struct passwd *pwd;
    pwd = getpwuid(getuid());
    strcpy(name, pwd->pw_name);
    
    get_conf_value(config, "MasterIp", master_ip);
    get_conf_value(config, "RealName", real_name);
    get_conf_value(config, "MasterPort", tmp);
    master_port = atoi(tmp);

    DBG("IP = %s Port=%d\n", master_ip, master_port);
    DBG("Name = %s RealNmae= %s\n", name, real_name);
    DBG("Config done.\n");

    if ((sockfd = connect_nonblock(master_port, master_ip, 30000)) < 0) {
        DBG("Can not connect to the server.\n");
        exit(1);
    }

    struct Msg msg;
    msg.type = 1;
    strcpy(msg.name, name);
    strcpy(msg.real_name, real_name);
    getcwd(msg.path, sizeof(msg.path));

    DBG("%s\n", msg.path);

    DBG("Sending msg to Server.\n");

    if (send(sockfd, (void *)&msg, sizeof(msg), 0) <= 0) {
        perror("send");
        exit(1);
    }
    DBG("Send success.\n");
    return 0;
}

