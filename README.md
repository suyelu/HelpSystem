# HelpSystem

> 实现了一个小的需求：在上课，或者平时，通过`Master`,`Teacher`、`Student`三端的协作，建立一条TCP隧道，可实现教师对学生机的快速连接。

## 安装

#### 对于`Student`端的安装

```shell
git clone https://github.com/suyelu/HelpSystem.git
cd HelpSystem/student
make
sudo make install
```

##  使用
> 前提，已完成Teacher到Student的免密登录，或告知Teacher密码
1. 在任何路劲下均可执行`helpme`命令，但远端教师会直接连接到**当前工作目录**

2. 目前支持`ctrl + c`结束程序



## 下一步计划

1. 添加线程锁，增加安全性
2. Student端如果在没有写权限的目录运行，会因创建文件没有权限而报段错误，可更新
3. Teacher到Student的免密登录，也可以在这个程序中实现
   1. 每一个Teacher在第一次上线后，将自己的公钥传给Master，Master根据是否已在本地保存该公钥决定是否保存
   2. Student上线后，需要在合适的实际，接收Master发来的Teacher的公钥
   3. 根据是否已认证决定是否完成免密登录的设置
   
