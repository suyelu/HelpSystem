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
int sockfd;
struct Code code;
void do_exit(int x) {
    char test_str[100] = {0};
        char cmd[100] = {0};
        sprintf(test_str, "helper-haizei%d", code.code);
        //退出时同时把tmux关掉
        sprintf(cmd, "tmux kill-session -t %s &", test_str);
        //printf("%s\n", cmd);
        system(cmd);
    printf("SSH-Tunnel closed.\n");
    printf("Bye.\n");
    close(sockfd);
    remove(key_file);
    exit(0);
}

unsigned long get_file_size(const char *path) {
    unsigned long filesize = -1;
    struct stat statbuff;
    if (stat(path, &statbuff) < 0){
        return filesize;
    } else {
        filesize = statbuff.st_size;
    }
    return filesize;
}

void send_file(int sockfd, char *filename) {
    FILE *fd = NULL;
    char data[1024] = {0};
    size_t num_read;
    fd = fopen(filename, "r");
    if (!fd) {
        DBG("File %s open error\n", filename);
    } else {
        uint64_t filesize = get_file_size(filename);

        if (send(sockfd, (void *)&filesize, sizeof(uint64_t), 0) <= 0) {
            DBG("File size send failed.\n");
            return ;
        }
        while (1) {
            num_read = fread(data, 1, 1024, fd);
            if (send(sockfd, data, num_read, 0) < 0) {
                DBG("Error in sending file.\n");
            }
            if (num_read == 0)  {
                break;
            }
            memset(data, 0, 1024);
        }
        DBG("%s sent sucess.\n", filename);
    }
    fclose(fd);
}

int get_file(int sockfd, char *filename) {
    char data[1024] = {0};
    int size;
    FILE *fp = fopen(filename, "w");
    int fd  =fileno(fp);
    fchmod(fd, 0600);
    unsigned long filesize = -1, total_size = 0;
    //DBG("len : %d\n", sizeof(uint64_t));
    int test_size = 0;
    while(test_size < (int)sizeof(uint64_t)) {
        test_size = recv(sockfd, (void *)&filesize, sizeof(uint64_t), 0);
        //DBG("test_size : %d\n",test_size);
        //DBG("filesize received : %d\n",filesize);
    }

    if (test_size <= 0) {
        DBG("File size recv failed.\n");
        return -1;
    }
    //printf("Recv file size = %ld!\n", filesize);
    while ((size = recv(sockfd, data, 1024, 0)) > 0) {
        //printf("%s\n", data);
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

int main() {
    int master_port, type = 1;
    char master_ip[20] = {0}, real_name[20] = {0}, name[20] = {0};
    char tmp[20] = {0}, home_dir[50] = {0};
    struct passwd *pwd;
    pwd = getpwuid(getuid());
    strcpy(name, pwd->pw_name);
    get_conf_value(config, "MasterIp", master_ip);
    get_conf_value(config, "RealName", real_name);
    get_conf_value(config, "HomeDir", home_dir);
    get_conf_value(config, "MasterPort", tmp);
    master_port = atoi(tmp);

    //DBG("Read Config Done.\n");

    if ((sockfd = connect_nonblock(master_port, master_ip, 90000)) < 0) {
        perror("Can not connect to the server");
        exit(1);
    }


    printf("Connected to Server.\n");

    if (send(sockfd, (void *)&type, sizeof(int), 0) <= 0) {
        perror("send type");
        exit(1);
    }

    if (send(sockfd, VER, sizeof(VER), 0) <= 0) {
        perror("send Version");
        exit(1);
    }

    int ver_flag;

    if (recv(sockfd, (void *)&ver_flag, sizeof(int), 0) <= 0) {
        perror("recv Version Flag");
        exit(1);
    }
    if (!ver_flag) {
        DBG("Version OK!\n");
    } else if (ver_flag == -1) {
        DBG("Version not OK!\n");
        close(sockfd);
        exit(1);
    } else {
        DBG("Version Flag Error \n");
        close(sockfd);
        exit(1);
    }

    struct Msg msg;
    //同一个云主机测试，改了个假名字
    // int fake_name = getpid();
    // sprintf(msg.name,"%d",fake_name);
    // printf("name : %s\n",msg.name);
    strcpy(msg.name, name);
    strcpy(msg.real_name, real_name);
    getcwd(msg.path, sizeof(msg.path));
    sprintf(key_file, "%s/id_rsa", msg.path);

    //DBG("Sending User-Msg to Server...\n");

    if (send(sockfd, (void *)&msg, sizeof(msg), 0) <= 0) {
        perror("send");
        close(sockfd);
        exit(1);
    }

    int online = -1;

    if (recv(sockfd, (void *)&online, sizeof(int), 0) <= 0) {
        perror("recv error");
        exit(1);
    }
    if (online == 1) {
        printf("You Are alread online somewhere, NO need do this.\n");
        exit(2);
    } else if (online == 0){
        printf("Login HelpSystem...\n");
    } else {
        printf("Something is wrong.\n");
        exit(2);
    }
    //Here we need recv for help code and port.

    int a = 0;
    if ((a = recv(sockfd, (void *)&code, sizeof(code), 0)) < 0) {
        perror("recv code");
        close(sockfd);
        exit(1);
    }



   //printf("After code recv!\n");
    //Here we need recv a id_rsa  prikey

    get_file(sockfd, key_file);
   //printf("After key_file recv!\n");

    //Here we send student's pubkey to Master, In order to giving it to teacher
    char pub_key[150] = {0};
    sprintf(pub_key, "%s/id_rsa.pub", msg.path);
    get_file(sockfd, pub_key);
   //printf("After puk_key recv!\n");


    char cmd_str[1024] = {0};
    sprintf(cmd_str, "check_key %s", pub_key);

    system(cmd_str);


    //以下多进程，在子进程中，开启ssh隧道
    //父进程等待
    char port_str[100];
    char user_str[100];
    sprintf(port_str, "%d:127.0.0.1:22", code.port);
    sprintf(user_str, "Helper@%s", master_ip);
    printf("Server has provide you a Help-Code : \033[31m %d\n\033[0m", code.code);
    printf("请记住Help-Code码，并在你寻求帮助时提供这个号码\n确认记住后请输入yes继续：");
    char str[100]={0};
    while(strcmp(str,"yes")!=0) {
        scanf("%s",str);
    }
    int pid;
    if ((pid = fork()) < 0) {
        perror("fork");
        close(sockfd);
        exit(1);
    }

        
    if (pid == 0) {
        close(sockfd);
        printf("Enter Ctrl+C terminate this.\n");
        fflush(stdout);
        int ret = execl("/usr/bin/ssh", "ssh", "-i", key_file ,"-p","10089","-N", "-R", port_str, user_str,"-o StrictHostKeyChecking = no","&", NULL);
        if (ret < 0) perror("excel");
        return 0;
    } else {
        //printf("In Father!\n");
        int pid1 ;
        if ((pid1 = fork()) < 0) {
            perror("fork");
            exit(1);
        }

        char test_str[100] = {0};
        char cmd[100] = {0};
        sprintf(test_str, "helper-haizei%d", code.code);
        if (pid1 == 0) {
            execlp("tmux", "tmux", "new-session", "-s", test_str, NULL);
        }

        signal(SIGINT, do_exit);
        //kill(getpid(),SIGINT);
        //printf("execpid = %d\n", pid);
        while (1) {
            int heart_beat;
            if (recv(sockfd, (void *)&heart_beat, sizeof(int), 0) <= 0) {
                close(sockfd);
                kill(pid, 9);
                remove(key_file);
                char cmd[100] = {0};
                char test_str[100] = {0};
                sprintf(test_str, "helper-%d", code.code);
                sprintf(cmd,"tmux kill-session -t s", test_str);
                DBG("cmd %s\n", cmd);
                system(cmd);
            }
        }
        wait(NULL);
    }

    return 0;
}
