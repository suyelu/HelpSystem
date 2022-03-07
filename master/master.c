/*************************************************************************
	> File Name: master.c
	> Author: suyelu
	> Mail: suyelu@haizeix.com
	> Created Time: 六  3/ 7 21:23:05 2020
 ************************************************************************/

#include "master.h"

char config[50] = "/etc/HelpSys/master.conf";
//char key_file[50] = "./id_rsa";
char key_pub[50] = "/etc/HelpSys/.ssh/id_rsa.pub";
char key_pri[50] = "/etc/HelpSys/.ssh/id_rsa";
//char config[50] = "./master.conf";
char tmp[20] = {0};
struct Stu student[MAX];
pthread_t client_t[MAX], teacher_t, heart_beat_t;
int client_fd[MAX], sub_index[MAX], teacher_fd[MAX], sub_index1[MAX], start_port;
int size, sum = 0, student_cnt = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int flag = 1;
bool check_online(char *realname, char *username) {
    for(int i = 0; i < size; i++) {
        if (student[i].flag && (strcmp(realname, student[i].real_name) == 0) && (strcmp(username, student[i].name) == 0)) {
            return true;
        }
    }
    return false;
}

unsigned long get_file_size(const char *path) {
    unsigned long filesize = -1;
    struct stat statbuff;
    if (stat(path, &statbuff) < 0){
        perror(errno);
        DBG("stat failed\n");
        return filesize;
    } else {
        filesize = statbuff.st_size;
    }
    return filesize;
}

void send_file(int sockfd, char *filename) {
    FILE *fp = NULL;
    char data[1024] = {0};
    size_t num_read;
    fp = fopen(filename, "r");
    DBG("scokfd : %d, fp : %d\n", sockfd, fp);
    if (fp == NULL) {
        DBG("File %s open error\n", filename);
    } else {
        uint64_t filesize = get_file_size(filename);
        DBG("filesize : %d\n", filesize);
        if (send(sockfd, (void *)&filesize, sizeof(uint64_t), 0) <= 0) {
            DBG("File size send failed.\n");
            return ;
        }
        while (1) {
            num_read = fread(data, 1, 1024, fp);
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
    fclose(fp);
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
int send_studentinfo(int ind, int help_code) {
    struct Msg_t msg_t;
    //同一个云主机测试，student端发给我的是假名字，这里改成真名
    //strcpy(student[help_code].name, "wanglu");
    strcpy(msg_t.name, student[help_code].name);
    strcpy(msg_t.real_name, student[help_code].real_name);
    strcpy(msg_t.path, student[help_code].path);
    msg_t.port = start_port + help_code;

    //Send Student's Information according to Help-Code

    int snum = 0;
    if ((snum = send(teacher_fd[ind], (void *)&msg_t, sizeof(msg_t), 0)) <= 0) {
        return -1;
    }
    return 1;
}
void *teacher_work(void *arg) {
    DBG("Teacher on.\n");
    int ind = *(int *)arg;
    if (send(teacher_fd[ind], (void *)&student_cnt, sizeof(int), 0) <= 0) {
        DBG("Number of online students  send failed.\n");
        return NULL;
    }
    int cnt = student_cnt;
    const int basenum = 10;
    cnt = cnt > basenum ? basenum : cnt;
    for (int i = 0; i < size && cnt > 0; i++)  {
        if (student[i].flag == true) {
            if(send_studentinfo(ind, i) < 0) {
                DBG("Send Student's Information Error.\n");
                close(teacher_fd[ind]);
                return NULL;
            }
            //发送学生收到的code码，可以和姓名一起同teacher端联系起来
            if (send(teacher_fd[ind], (void *)&i, sizeof(int), 0) <= 0) {
                DBG("Code of online students  send failed.\n");
                return NULL;
            }
            cnt--;
        }
    }
    // int help_code = -1;
    // //Recv Help-Code
    // if (recv(teacher_fd[ind], (void *)&help_code, sizeof(int), 0) <= 0) {
    //     DBG("Recv Help-Code Error.\n");
    //     close(teacher_fd[ind]);
    //     return NULL;
    // }

    //char key_pub[150] = {0};
    //sprintf(key_pub, "/tmp/help_%d.tmp", help_code);
    send_file(teacher_fd[ind], key_pri);

    close(teacher_fd[ind]);
    //防止多线程带来的对当前teacher_fd[ind]修改问题
    student[ind].flag = false;

    return NULL;
}

int version_ctrl(int fd) {
    char version[20] = {0};
    if (recv(fd, version, sizeof(version), 0) <= 0) {
        perror("recv version code");
        return -1;
    }
    if (strcmp(VER, version)) return -1;
    return 0;
}

static void cleanupHandler(void *arg) {
    printf("unlock");
    fflush(stdout);
    pthread_mutex_unlock(&lock);
}
void *work(void *arg) {
    int ind = *(int*)arg;
    DBG("%d\n", ind);
    int ret = version_ctrl(client_fd[ind]);
    send(client_fd[ind], (void *)&ret, sizeof(int), 0);
    if (ret < 0) {
        student[ind].flag = false;
        close(client_fd[ind]);
        return NULL;
    }
    struct Msg first_msg;
    if (recv(client_fd[ind], (void *)&first_msg, sizeof(first_msg), 0) <= 0) {
        perror("recv");
        student[ind].flag = false;
        close(client_fd[ind]);
        return NULL;
    }
    int online = 0;
    if (check_online(first_msg.real_name, first_msg.name)) {
       //名字重复
        DBG("Already on system.\n");
        student[ind].flag_using = false ;
        student[ind].flag = false;
        online = 1;
        send(client_fd[ind], (void *)&online, sizeof(int), 0);
        close(client_fd[ind]);
        return NULL;
    } else {
        send(client_fd[ind], (void *)&online, sizeof(int), 0);
        student[ind].flag = true;
    }
    pthread_mutex_lock(&lock);
    student_cnt += 1;
    pthread_mutex_unlock(&lock);
    student[ind].flag_using = false ;
    //添加信息到列表
    strcpy(student[ind].name, first_msg.name);
    strcpy(student[ind].path, first_msg.path);
    strcpy(student[ind].real_name, first_msg.real_name);
    DBG("%s:%s:%s\n", student[ind].real_name, student[ind].name, student[ind].path);
    //Here we send student about Help-Code and Port.
    struct Code code;
    code.code = ind;
    code.port = start_port + ind;

    if (send(client_fd[ind], (void *)&code, sizeof(code), 0) <= 0) {
        DBG("Send Help-Code failed.\n");
        close(client_fd[ind]);
        student[ind].flag = false;
        return NULL;
    }

    printf("send code!\n");
    //Now we need seed a id_rsa pri key to student in order to ssh

    send_file(client_fd[ind], key_pri);
/*
    char save_file[50] = {0};
    sprintf(save_file, "/tmp/help_%d.tmp", ind);
    printf("save_file = %s\n", save_file);
    get_file(client_fd[ind], save_file);
*/

    sleep(2);
    printf("key_pub = %s\n", key_pub);
    send_file(client_fd[ind], key_pub);

    pthread_cleanup_push(cleanupHandler, NULL);
    printf("After send key_pub\n");
    if (recv(client_fd[ind], (void *)&first_msg, sizeof(first_msg), 0) <= 0) {
        printf("ooooooooooooooooooo");
        fflush(stdout);
        pthread_mutex_lock(&lock);
        student_cnt -= 1;
        pthread_mutex_unlock(&lock);
        DBG("Client closed.\n");
        close(client_fd[ind]);
        student[ind].flag = false;
        return NULL;
    }
    pthread_cleanup_pop(1);

    return NULL;
}
void * heart_beat(void* p){
    //心跳过程
    while (1) {
        sleep(10);
        int heart_beat = 1;
        for (int ind = 0; ind < MAX; ind++){
            if(student[ind].flag == false) continue;
            if (send(client_fd[ind], (void *)&heart_beat, sizeof(int), 0) <= 0) {
               //Student already dead
                pthread_mutex_lock(&lock);
                student_cnt -= 1;
                pthread_mutex_unlock(&lock);
                pthread_cancel(client_t[ind]);
                close(client_fd[ind]);
                student[ind].flag = false;
                int s = pthread_cancel(client_t[ind]);
                if(s != 0) {
                    perror("pthread_cancel");
                    exit(-1);
                } 
            }
        }
    }
    return NULL;
}
int main() {
    
    signal(SIGPIPE,SIG_IGN);
    int  master_port,  master_listen, client_in;
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
    pthread_create(&heart_beat_t, NULL,heart_beat,NULL);

    while (1) {
        //DBG("Master listening.\n");
        if ((client_in = accept(master_listen, NULL, NULL)) < 0) {
            printf("-----------------");
            fflush(stdout);
            perror("accept()");
            exit(1);
        }
        struct timeval tv_out;
        tv_out.tv_sec=5*60;
        tv_out.tv_usec=0;
 //setsockopt(client_in, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out));
        printf("sockfd : %d\n", client_in);
        int sub = -1;
        for (int i = 0; i < size; i++)  {
            if (student[i].flag == false && student[i].flag_using == false) {
                student[i].flag_using = true;
                sub = i;
                break;
            }
        }
        sum++;
        if (sub < 0) {
            //服务端端口已全部用完
            //通知客户端无法建立连接，断开连接，并进入下次循环
            close(client_in);
            continue;
        }
        //student[sub].flag = true;
        client_fd[sub] = client_in;
        sub_index[sub] = sub;
        int who = -1;
        if (recv(client_in, (void *)&who, sizeof(int), 0) <= 0) {
            DBG("User type unknown.\n");
            student[sub].flag = false;
            close(client_in);
            continue;
        }
        if (who == 1)
            pthread_create(&client_t[sub], NULL, work, (void *)&sub_index[sub]);
        else if (who == 0) {
            student[sub].flag_using=false;
            teacher_fd[sub] = client_in;
            sub_index1[sub] = sub;
            pthread_create(&teacher_t, NULL, teacher_work, (void *)&sub_index1[sub]);
        }
        else {
            DBG("User type unknown.\n");
            student[sub].flag = false;
            close(client_in);
            continue;
        }
    }

    return 0;
}
