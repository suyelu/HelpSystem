# HelpSystem

> 实现了一个小的需求：在上课，或者平时，通过`Master`,`Teacher`、`Student`三端的协作，建立一条TCP隧道，可实现教师对学生机的快速连接。

## 安装

#### 对于`Student`端的安装

```shell
cd student
make
sudo make install
```

##  使用

1. 在任何路劲下均可执行`helpme`命令，但远端教师会直接连接到**当前工作目录**

2. 目前支持`ctrl + c`结束程序



## 下一步计划

1. 添加线程锁，增加安全性

   