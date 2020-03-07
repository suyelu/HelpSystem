#include "common.h"


int get_conf_value(char* pathname, const char* key_name, char *value) {

	FILE *fp = NULL;
	char *line = NULL, *substr = NULL;
	size_t len = 0, tmplen = 0;
	ssize_t read;
	//memset(value, 0, sizeof(char)*MAX_SIZE);

	if ( key_name == NULL || value == NULL) {
		DBG("paramer is invaild!\n");
		exit(-1);
	}

	fp = fopen(pathname,"r");
	if (fp == NULL) {
		DBG("Open config file error!\n");
		exit(-1);
	}

	while (( read = getline(&line, &len,fp)) != -1) {
		substr = strstr(line, key_name);
		if (substr == NULL) 
			continue;
		else {
			tmplen = strlen(key_name);
			if (line[tmplen] == '=') {
				strncpy(value, &line[tmplen + 1], (int)read - tmplen + 1);
				tmplen = strlen(value);
				*(value + tmplen - 1) = '\0';
				break;
			}
			else {
				DBG("Maybe there is something wrong with config file!\n");
				continue;
			}
		}
	}

	if (substr == NULL) {
		DBG("%s not found in config file!\n", key_name);
		fclose(fp);
		exit(-1);
	}

	//DBG("%s=%s\n", key_name, value);
	free(line);
	fclose(fp);
	return 0;
}



/*
*创建监听套接字
*返回套接字描述符，错误返回-1
*/

int socket_create(int port){
	int sockfd;
	int yes = 1;
	struct sockaddr_in sock_addr;

	struct linger m_sLinger;
	m_sLinger.l_onoff = 1;
	m_sLinger.l_linger = 0;
	

	//创建套接字
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket() error");
		return -1;
	}

	//设置本地套接字地址
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(port); //转化为网络字节序
	sock_addr.sin_addr.s_addr = htonl(INADDR_ANY); //0.0.0.0

	//设置本地套接字
	setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (const char*)&m_sLinger,sizeof(struct linger));

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		close(sockfd);
		perror("setsockopt() error\n");
		return -1;
	}

	//绑定本地套接字到套接字
	if (bind(sockfd, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) < 0) {
		close(sockfd);
		perror("bind() error");
		return -1;
	}

	//将套接字设置为监听状态
	if (listen(sockfd, 20) < 0) {
		close(sockfd);
		perror("listen() error");
		return -1;
	}
	return sockfd;
}




/*
*接受套接字请求
*返回新的套接字描述符，错误返回-1
*/



int connect_nonblock(int port, char *host, long timeout) {
	int sockfd;
	struct sockaddr_in dest_addr;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket() error");
		return -1;
	}

	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);
	dest_addr.sin_addr.s_addr = inet_addr(host);



	int error = -1, len;
	len = sizeof(int);
	struct timeval tm;
	fd_set set;
	unsigned long ul = 1;
	ioctl(sockfd, FIONBIO, &ul);

	bool ret = false; 
	//fflush(stdout);
	if (connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
		tm.tv_sec = 0;
		tm.tv_usec = timeout;
		FD_ZERO(&set);
		FD_SET(sockfd, &set);
		if( select(sockfd+1, NULL, &set, NULL, &tm) > 0)
		{
			getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
			if(error == 0) {
				ret = true;
			}
			else ret = false;
		} 
		else ret = false;		
	}
    if (!ret) {
        close(sockfd);
        return -1;
    }
	ul = 0;
	ioctl(sockfd, FIONBIO, &ul);
    return sockfd;
}


bool check_connect(struct sockaddr_in addr, long timeout){
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        DBG("%s\n", strerror(errno));
        return false;
    }
    int error = -1, len;
	len = sizeof(int);
	struct timeval tm;
	fd_set set;
	unsigned long ul = 1;
	ioctl(sockfd, FIONBIO, &ul);

	bool ret = false;
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		tm.tv_sec = 0;
		tm.tv_usec = timeout;
		FD_ZERO(&set);
		FD_SET(sockfd, &set);
		if( select(sockfd + 1, NULL, &set, NULL, &tm) > 0)
		{
			getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
			if(error == 0) {
				ret = true;
			}
			else ret = false;
		} 
		else ret = false;		
    } else {
    	ret = true;
    }

    close(sockfd);
    return ret;
}
