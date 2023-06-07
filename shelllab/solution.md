# shell-lab

手写一个 shell 程序，说实话上完了 lecture 14,15,16 后发现 shell-lab 的完成度可以比我原本预想中更高。

## 实验要求

还是先读一遍 writeup

我们要完成的内容全部在 tsh.c 里面，大体程序框架已经写好了，我们只需要专注于实现其中关键的几个功能。

![image-20230528123849270](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bedimage-20230528123849270.png)

`eval`:  解释命令的主例程

`builtin_cmd`: 识别并解释内置命令:` quit`, `fg`,` bg`, and  `jobs`

`do_bgfg`: 实现 `bg` 和 `fg` 这两个内置命令

`waitfg`: 等待一个前台任务完成

`sigchld_handler`: 捕获 `SIGCHILD` 信号

`sigint_handler`: 捕获 `SIGNINT`(ctrl-c) 信号

`sigtstp_handler`: 捕获 `SIGTSTP` (ctrl-z) 信号

![image-20230528152646344](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bedimage-20230528152646344.png)

## 实验建议

其实每次 lab 在 WriteUp 里都有教授非常详细贴心的提示，虽然之前每次都因为不想读英文选择性忽视了，但这次还是读一下再开始的好。

如果输入的是 builtin command 就立刻在当前进程执行。如果不是的话将其视作一个可执行文件的 pathname，在这种情况下，shell 进程会 fork 一个子进程，然后在子进程通过系统调用中加载并执行这个程序。作为命令执行的结果而创建的子进程被称作 `job`，一个 job 可以包含使用 Unix 管道符连接起来的多个进程（例如： `ps | grep docker` 就包含了 ps 和 grep 两个进程）。

如果命令以 `&` 结尾，那么 job 将在后台运行，这意味着 shell 进程将不会等待 job 终止并且立刻开始等待下一个命令。相反的，如果 shell 进程在前台运行，shell 进程会等待 job 终止才会开始等待下一个命令。当然，同一时间只能有一个 job 跑在前台，但可以有多个 job 跑在后台。

Unix Shell 支持 Job Control 的概念，即允许用户将 job 在前台与后台之间移动，改变 job 中进程的状态 （running, stopped, terminated）。输入 ctrl-c 可以发送 SIGINT 信号给当前前台 job 的所有进程，SIGINT 的默认行为是终止进程。同样的，输入 ctrl+z 可以发送 SIGTSTP 信号，SIGTSTP 的默认行为是将进程置于 stopped 状态，即一直保留直到被 SIGCONT 信号唤醒。Unix Shell 也提供了多种内置命令来支持 Job Control，例如：

- `jobs` 列出正在运行的和 stopped 的后台 job
- bg <job\>: 将一个 stopped 的后台进程唤醒
- fg <job\>: 将一个后台进程移动到前台
- kill <job\>: 终止一个 job

