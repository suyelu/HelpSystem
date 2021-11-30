/*************************************************************************
	> File Name: master.h
	> Author: suyelu
	> Mail: suyelu@haizeix.com
	> Created Time: 六  3/ 7 21:27:34 2020
 ************************************************************************/

#ifndef _MASTER_H
#define _MASTER_H
#define MAX 1000
#include "../common/common.h"


struct Stu {
    bool flag;
    //防止不同客户端用一个数组下标记录,导致发送给他们同一个code码
    bool flag_using;
    char name[20];
    char real_name[20];
    char path[50];
};

#endif
