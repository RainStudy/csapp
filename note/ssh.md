# 在 ubuntu 中安装 openssh server

先更新一下

~~~shell
sudo apt update
~~~

然后安装 openssh-server

~~~shell
sudo apt install openssh-server
~~~

然后启动 sshd 服务

~~~shell
sudo /etc/init.d/ssh start
~~~

然后检查服务是否已经启动成功

~~~shell
ps -e | grep ssh # 找到sshd进程就说明已经部署完成
~~~

ssh 服务默认端口为22，可以将其映射到其他端口

然后用ssh命令登录验证是否部署成功 （windows 使用 powershell, *nix 使用终端即可）

~~~shell
ssh <user>@<ip> -p <port> # 端口在为22时可以不写
~~~

