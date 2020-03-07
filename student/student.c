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
char key_file[50] = "/tmp/id_rsa";


int get_file(int sockfd, char *filename) {
    char data[1024];
    int size;
    FILE *fp = fopen(filename, "w");
    unsigned long filesize = -1, total_size = 0;
    if (recv(sockfd, (void *)&filesize, sizeof(unsigned long), 0) <= 0) {
        DBG("File size recv failed.\n");
        return -1;
    }
    while ((size = recv(sockfd, data, 1024, 0)) > 0) {
        fwrite(data, 1, size, fp);
        DBG("%s", data);
        total_size += size;
        if (total_size >= filesize) {
            DBG("File finished.\n");
            break;
        }
    }
    fclose(fp);
    DBG("File write to %s\n", filename);
    return 0;
}

int main() {
    int master_port, sockfd, type = 1;
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
    
    
    if (send(sockfd, (void *)&type, sizeof(int), 0) <= 0) {
        perror("send type");
        exit(1);
    }
    
    DBG("Sent User-Type to Server.\n");

    struct Msg msg;
    strcpy(msg.name, name);
    strcpy(msg.real_name, real_name);
    getcwd(msg.path, sizeof(msg.path));

    DBG("%s\n", msg.path);

    DBG("Sending msg to Server.\n");

    if (send(sockfd, (void *)&msg, sizeof(msg), 0) <= 0) {
        perror("send");
        close(sockfd);
        exit(1);
    }
    DBG("Send success.\n");
    

    //Here we need recv for help code and port.
    struct Code code;

    if (recv(sockfd, (void *)&code, sizeof(code), 0) < 0) {
        perror("recv code");
        close(sockfd);
        exit(1);
    }

    DBG("Server has provide you a Help-Code : %d\n", code.code);
    DBG("Please Tell Your Teacher This Help-Code %d\n", code.code);
    
    //Here we need recv a id_rsa key
    
    get_file(sockfd, key_file);

    int pid;

    if ((pid = fork()) < 0) {
        perror("fork");
        close(sockfd);
        exit(1);
    }

    //以下多进程，在子进程中，开启ssh隧道
    //父进程等待
    return 0;
}

