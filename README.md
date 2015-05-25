#MyShell
---
##功能概述

一个简单的shell程序。实现了基本功能，包括一些内部命令，所有外部命令，支持标准IO重定向和管道。但是IO重定向和管道只能适用于简单的情况。

平台: Ubuntu14.04 </br>
运行MyShell即可进入shell，会出现`[MyShell] zdf@zdf-virtual-machine:~/MyShell$`类似这样的提示

###支持的内部命令

* _cd_  与linux下的cd功能相同：
</br>cd .. 返回上一级目录 
</br>cd 进入个人的主目录
</br>cd ~/directory 进入个人主目录下的目录
</br>cd directory 进入目录
* _exit/quit_  退出shell

实现此部分所使用的系统调用有`getpwuid`、`getuid`和`chdir`。

###支持所有外部命令

支持所有的外部命令，且支持参数

实现此部分所使用的系统调用有`execvp`。

###支持标准IO重定向

只能对第一条命令做重定向

实现此部分所使用的系统调用有`open`和`dup2`。

###支持管道

只能通过管道连接两个命令，例如`ls -l | sort`。所以一条命令中只能有一个`|`。

实现此部分所使用的系统调用有`fork()`、`pipe()`和`dup2`。

###其他

当输出重定向与管道同时存在时，管道命令只能放在最后，例如`ls -l > tmp | sort`。但是此时管道已经没有意义了。所以不建议输出重定向与管道共用。