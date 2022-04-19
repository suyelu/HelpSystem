/*************************************************************************
	> File Name: teacher.c
	> Author: suyelu
	> Mail: suyelu@haizeix.com
	> Created Time: 日  3/ 8 05:18:05 2020
 ************************************************************************/

#include "teacher.h"

char config[50] = "/etc/HelpSys/teacher.conf";
//char config[50] = "./teacher.conf";


int get_file(int sockfd, char *filename) {
    char data[1024] = {0};
    int size;
    FILE *fp = fopen(filename, "w");
    int fd  = fileno(fp);
    fchmod(fd, 0600);
    uint64_t filesize = -1, total_size = 0;
    if (recv(sockfd, (void *)&filesize, sizeof(uint64_t), 0) <= 0) {
        DBG("File size recv failed.\n");
        perror("recv");
        return -1;
    }
   // printf("RecvSize = %d\n", filesize);
    while ((size = recv(sockfd, data, 1024, 0)) > 0) {
       // printf("Recv = %s\n", data);
        fwrite(data, 1, size, fp);
        total_size += size;
        if (total_size >= filesize) {
            break;
        }
        memset(data, 0, 1024);
    }
    fclose(fp);
    return 0;
}

int main(int argc, char **argv) {

    int master_port, sockfd, type = 0, code = 0;
    char master_ip[20] = {0}, tmp[20] = {0};
    get_conf_value(config, "MasterIp", master_ip);
    get_conf_value(config, "MasterPort", tmp);
    master_port = atoi(tmp);

   // printf("MasterIP = %s\nMasterPort=%d\n", master_ip, master_port);
    if ((sockfd = connect_nonblock(master_port, master_ip, 800000)) < 0) {
       // printf("sockfd = %d\n",sockfd);
        perror("socket");
        DBG("Connect to Server Error.\n");
        close(sockfd);
        exit(1);
    }

    if (send(sockfd, (void *)&type, sizeof(int), 0) <= 0) {
        DBG("Send User-Type Error.\n");
        close(sockfd);
        exit(1);
    }

    printf("Show at most 10 online students info: \n");
    //Recv for Student's Information.
    int online_cnt = 0;
    if (recv(sockfd, (void *)&online_cnt, sizeof(int), 0) <= 0) {
        DBG("Number of online students recv failed.\n");
        perror("recv");
        return -1;
    }
    printf("Total number of online students : %d\n", online_cnt);
    int num = 10;
    num = num > online_cnt ? online_cnt : num;
    struct Msg_t students[300];

    while(num--) {
        struct Msg_t msg_t;
        if (recv(sockfd, (void *)&msg_t, sizeof(msg_t), 0) <= 0) {
            DBG("Recv Student's Information Error.\n");
            close(sockfd);
            exit(1);
        }
        int code = 0;
        if (recv(sockfd, (void *)&code, sizeof(int), 0) <= 0) {
            DBG("Code of online students recv failed.\n");
            perror("recv");
            return -1;
        }
        DBG("code : %d\n",code);
        students[code] = msg_t;
        printf("name : %s\nreal_name : %s\npath : %s\nport : %d\n", msg_t.name,msg_t.real_name,msg_t.path,msg_t.port);
        printf("=====================\n");
    }
    printf("Please choose one student and input his or her code\n");
    //Send Help-Code to Server
    scanf("%d", &code);
    struct Msg_t student_choosed = students[code];
    char key_file[50] = {0};
    sprintf(key_file, "./%s_%d_%d.tmp", student_choosed.name, student_choosed.port, code);
    get_file(sockfd, key_file);

    close(sockfd);

    DBG("Connecting to %s's system...\n", student_choosed.real_name);

    //Now we have student's information, we need connect to student.

    int pid;

    if ((pid = fork()) < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        char user_str[50], cmd_str[100], port_str[20];
        sprintf(user_str, "%s@%s", student_choosed.name, master_ip);
        sprintf(cmd_str, "/usr/local/bin/tmux attach-session -t helper-haizei%d -c %s", code,student_choosed.path);
        DBG("cmd_str : %s", cmd_str);
        sprintf(port_str, "%d", student_choosed.port);
        int ret = execl("/usr/bin/ssh", "ssh", "-i", key_file, "-o StrictHostKeyChecking no", "-p", port_str, user_str, "-t", cmd_str, NULL);
        if (ret < 0) {
            perror("execl");
        }
        return 0;
    } else {
        wait(NULL);
    }
    return 0;
}
