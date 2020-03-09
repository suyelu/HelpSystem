/*************************************************************************
	> File Name: student.c
	> Author: suyelu
	> Mail: suyelu@haizeix.com
	> Created Time: 日  3/ 8 00:47:57 2020
 ************************************************************************/

#include "student.h"
#include <pwd.h>

char config[50] = "/etc/HelpSys/student.conf";
//char config[50] = "./student.conf";
char key_file[50];


int get_file(int sockfd, char *filename) {
    char data[1024];
    int size;
    FILE *fp = fopen(filename, "w");
    int fd  =fileno(fp);
    fchmod(fd, 0600);
    unsigned long filesize = -1, total_size = 0;
    if (recv(sockfd, (void *)&filesize, sizeof(unsigned long), 0) <= 0) {
        DBG("File size recv failed.\n");
        return -1;
    }
    while ((size = recv(sockfd, data, 1024, 0)) > 0) {
        fwrite(data, 1, size, fp);
        total_size += size;
        if (total_size >= filesize) {
            break;
        }
    }
    fclose(fp);
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

    DBG("Read Config Done.\n");

    if ((sockfd = connect_nonblock(master_port, master_ip, 90000)) < 0) {
        perror("Can not connect to the server");
        exit(1);
    }
    
    
    printf("Connected to Server.\n");

    if (send(sockfd, (void *)&type, sizeof(int), 0) <= 0) {
        perror("send type");
        exit(1);
    }
    

    struct Msg msg;
    strcpy(msg.name, name);
    strcpy(msg.real_name, real_name);
    getcwd(msg.path, sizeof(msg.path));
    sprintf(key_file, "%s/.id_rsa", msg.path);

    DBG("Sending msg to Server...\n");

    if (send(sockfd, (void *)&msg, sizeof(msg), 0) <= 0) {
        perror("send");
        close(sockfd);
        exit(1);
    }

    //Here we need recv for help code and port.
    struct Code code;

    if (recv(sockfd, (void *)&code, sizeof(code), 0) < 0) {
        perror("recv code");
        close(sockfd);
        exit(1);
    }

    
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
    
    if (pid == 0) {
        close(sockfd);
        char port_str[100];
        char user_str[100];
        sprintf(port_str, "%d:127.0.0.1:22", code.port);
        sprintf(user_str, "Helper@%s", master_ip);
        printf("Server has provide you a Help-Code : %d\n", code.code);
        printf("Enter Ctrl+C terminate this.");
        fflush(stdout);
        int ret = execl("/usr/bin/ssh", "ssh", "-i",key_file ,"-N", "-R", port_str, user_str, NULL);
        if (ret < 0) perror("excel");
        return 0;
    } else {
        wait(NULL);
        close(sockfd);
        remove(key_file);
    }

    return 0;
}

