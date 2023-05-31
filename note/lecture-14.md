# Exceptional Control Flow Exceptions and Processes

> 因为是看到一半才想起来应该记点笔记的，所以控制流那部分没有笔记
>
> 反正之后也会看书的（

## Control Flow

> event 可以理解为状态机中一次 state 的变化，状态机的状态流转是由事件驱动的

## Process

> ![0f1627cd-2718-446d-97b5-8a6c6e9e90eb](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bed0f1627cd-2718-446d-97b5-8a6c6e9e90eb.jpeg)
>
> 操作系统对进程做了一个抽象，使我们程序的进程似乎完全拥有机器上所有寄存器的所有权。实际上在进程上下文切换的过程中操作系统会将寄存器值写入内存进行保存，再读取其他进程保存在内存中的寄存器值。
>
> 在发生上下文切换时进程会收到一个中断信号，中断控制流的运行。
>
> 所谓的进程上下文切换就是地址空间和寄存器的变化，进程上下文切换的开销基本就在读写内存中保存的寄存器值上。
>
> 地址空间指为每个进程分配的内存空间（堆，栈等）

> “单核为什么能并发” 这个问题的解答
>
> ![image-20230520163901932](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bedimage-20230520163901932.png)
>
> 操作系统会在进程执行一小段时间后中断该进程的控制流，将其移交给其他进程的控制流，同时进行进程上下文的切换。
>
> 从宏观的时间尺度来看，这些进程是并行执行的。实际上从微观的角度上看是并发执行的（只要在进程开始到进程结束时间线上存在重叠的部分就可以说这两个进程是并发执行的，如上图，A和B是并发的，但B和C是串行的）

> ![image-20230520164830999](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bedimage-20230520164830999.png)
>
> 实际上进程进行上下文切换的过程是由 kernel code 控制的。所以实际上在发生进程上下文切换时切换到了 kernel code 的控制流（也就是说其实上下文切换是在内核态进行的），进行上下文切换之后再切换到下一个进程（内核调用它的调度程序来决定是继续当前进程还是切换到下一个进程）的控制流。

### fork

> fork 本质上就是 copy 了父进程的地址空间。fork 出来的子进程获得了跟父进程相同的 open file descriptors，所以它虽然跟父进程不是一个进程，是分隔开的，但其仍然可以访问父进程打开的文件，包括父进程的 stdin 和 stdout
>
> 既然复制的内容一模一样，那么想必程序计数器`%rip`指向的地址也是相同的吧，没错，复制出来的子进程会跟父进程的程序计数器指向同一行代码，但由于是复制过来的，两个进程所持有的内存空间和寄存器又是独立的。
>
> ~~~c
> #include <stdio.h>
> #include <unistd.h>
> #include <stdlib.h>
> 
> int main() {
> 	int pid;
> 	int x = 1;
> 	pid = fork();
> 	if (pid == 0) {
>  		// child
> 		printf("child : x=%d\n", ++x);
>  		exit(0);
> 	}
> 	// parent
> 	printf("parent: x=%d\n", --x);
> 	return 0;
> }
> ~~~
>
> ~~~
> unix > ./fork
> child : x=2
> parent : x=0
> ~~~
>
> ~~~c
> #include <stdio.h>
> #include <unistd.h>
> #include <stdlib.h>
> 
> int main() {
>   printf("L0\n");
>   fork();
>   printf("L1\n");
>   fork();
>   printf("Bye\n");
> }
> 
> ~~~

## routine

其实 routine 就是一个函数，或者一个方法。为什么协程要叫 coroutine？go 的协程为什么又要叫 goroutine？用过 goroutine 的人都知道，goroutine 用法实际上就是在函数调用之前加一个 go 罢了，而且也有相当一部分协程的实现是 async/await。js可以直接运行 async function，这本质上就是启动了一个协程。那么就非常好理解了，协程最开始的意义其实就是一个支持协作式调度的 routine，从这方面来说我觉得协程这个让人非常容易联想到线程的翻译其实是不太好的，很多人把协程简单的解释为 “轻量级线程” 更是加重了很多初学者的误解。

那么 kotlin 中 coroutine 的体现又是什么呢？那当然是挂起函数，它是一个 suspendable routine，也就是可以挂起，可以挂起那自然意味着可以实现协作式调度。只不过我们必须要在所谓的协程作用域中才能调用挂起函数，也就是说，顶部要有一个 CoroutineScope#launch 出来的顶部协程 （CoroutineScope#launch 其实就是将靠编译期魔法实现的 coroutine（就是挂起函数）交给异步运行时进行协程的调度）。当然 CoroutineScope 中 launch 出来的那个 lambda 也是 coroutine，它也是一个挂起函数，不过是一个匿名函数罢了。