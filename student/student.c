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

void do_exit(int x) {
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
    if (recv(sockfd, (void *)&filesize, sizeof(uint64_t), 0) <= 0) {
        DBG("File size recv failed.\n");
        return -1;
    }
    while ((size = recv(sockfd, data, 1024, 0)) > 0) {
        printf("%s", data);
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
        DBG("Version Flag Error\n");
        close(sockfd);
        exit(1);
    }

    struct Msg msg;
    strcpy(msg.name, name);
    strcpy(msg.real_name, real_name);
    getcwd(msg.path, sizeof(msg.path));
    sprintf(key_file, "%s/.id_rsa", msg.path);

    DBG("Sending User-Msg to Server...\n");

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
    struct Code code;
    int a = 0;
    if ((a = recv(sockfd, (void *)&code, sizeof(code), 0)) < 0) {
        perror("recv code");
        close(sockfd);
        exit(1);
    }

    

   printf("After code recv!\n"); 
    //Here we need recv a id_rsa  prikey 
    
    get_file(sockfd, key_file);
   printf("After key_file recv!\n"); 

    //Here we send student's pubkey to Master, In order to giving it to teacher
    char pub_key[150] = {0};
    sprintf(pub_key, "%s/id_rsa.pub", msg.path);
    get_file(sockfd, pub_key);
   printf("After puk_key recv!\n"); 

    printf("before fork()\n");
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
        printf("Enter Ctrl+C terminate this.\n");
        fflush(stdout);
        int ret = execl("/usr/bin/ssh", "ssh", "-i", key_file ,"-N", "-R", port_str, user_str, "&", NULL);
        if (ret < 0) perror("excel");
        return 0;
    } else {
        printf("In Father!\n");
        int pid1 ;
        if ((pid1 = fork()) < 0) {
            perror("fork");
            exit(1);
        } 
        if (pid1 == 0) {
            execlp("tmux", "tmux", "new-session", "-s", "helper-haizei", NULL);
        }
        signal(SIGINT, do_exit);
        printf("execpid = %d\n", pid);
        while (1) {
            int heart_beat;
            if (recv(sockfd, (void *)&heart_beat, sizeof(int), 0) <= 0) {
                close(sockfd);
                kill(pid, 9);
                remove(key_file);
                system("tmux kill-session -t helper-haizei");
            }
        }
        wait(NULL);
    }

    return 0;
}

