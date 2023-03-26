# solution

## phase_1

`phase_1` 函数有一个参数，为我们输入的字符串（字符数组），它被存储在 `%rdi` 寄存器

~~~assembly
0000000000400ee0 <phase_1>:
  400ee0:	48 83 ec 08          	sub    $0x8,%rsp # %rsp-8 -> %rsp 入栈，调整栈指针
  400ee4:	be 00 24 40 00       	mov    $0x402400,%esi # 将字符串常量地址存入 %esi 寄存器，作为函数的第二个参数 那么只要找到 0x402400 这个地址对应的字符串是什么这题就破了
  400ee9:	e8 4a 04 00 00       	call   401338 <strings_not_equal> # 调用函数判断字符串是否不一致
  400eee:	85 c0                	test   %eax,%eax # 自己与一下,如果与出来为0（即strings_not_equal返回的结果为0) 则设置零标志位ZF为1
  400ef0:	74 05                	je     400ef7 <phase_1+0x17> # 如果ZF为1则跳转到 400ef7
  400ef2:	e8 43 05 00 00       	call   40143a <explode_bomb> # bomb!
  400ef7:	48 83 c4 08          	add    $0x8,%rsp # %rsp+8 -> %rsp 出栈
  400efb:	c3                   	ret
~~~

用 gdb 看看这个字符串是什么

![image-20230326002208939](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bed/image-20230326002208939.png)

拿到第一个密钥

> Border relations with Canada have never been better.

## phase_2

phase_2 中栈的使用比较多

> 被调用者保存寄存器是指在函数调用时，被调用者需要保存一些寄存器的值，以便在函数返回时恢复这些寄存器的值。在x86_64架构中，被调用者保存寄存器包括%rbp、%rbx和%r12~%r15。
>
> push 操作就是保存寄存器的值，在 pop 的时候恢复
>
> 一般进入函数后会先 push 需要使用的被调用者保存寄存器，并且在 ret 前恢复

~~~assembly
0000000000400efc <phase_2>:
  400efc:	55                   	push   %rbp # 被调用者保存寄存器
  400efd:	53                   	push   %rbx # 被调用者保存寄存器
  400efe:	48 83 ec 28          	sub    $0x28,%rsp # 开辟一块0x28大小的栈空间
  400f02:	48 89 e6             	mov    %rsp,%rsi # 将开辟出来的空间的起始地址作为第二个参数
  400f05:	e8 52 05 00 00       	call   40145c <read_six_numbers> # 将输入的字符串和栈指针作为参数调用 read_six_number 函数，返回值为一个int*
  400f0a:	83 3c 24 01          	cmpl   $0x1,(%rsp) # 取上面开辟的栈空间的地址解引用跟1比较 (这里可以推出上面应该开辟了一块int数组)
  400f0e:	74 20                	je     400f30 <phase_2+0x34> # 条件不通过，没有跳转就爆炸
  400f10:	e8 25 05 00 00       	call   40143a <explode_bomb>
  400f15:	eb 19                	jmp    400f30 <phase_2+0x34>
  400f17:	8b 43 fc             	mov    -0x4(%rbx),%eax # 400f2c 400f3a 跳转到这里, eax = *(rbx-4)
  400f1a:	01 c0                	add    %eax,%eax # eax += eax
  400f1c:	39 03                	cmp    %eax,(%rbx) # eax == *(rbx) 这里即可推出每一个数为前面的两倍
  400f1e:	74 05                	je     400f25 <phase_2+0x29> # 条件不通过，没有跳转就爆炸
  400f20:	e8 15 05 00 00       	call   40143a <explode_bomb>
  400f25:	48 83 c3 04          	add    $0x4,%rbx
  400f29:	48 39 eb             	cmp    %rbp,%rbx
  400f2c:	75 e9                	jne    400f17 <phase_2+0x1b> # 
  400f2e:	eb 0c                	jmp    400f3c <phase_2+0x40> # 从循环中跳出
  400f30:	48 8d 5c 24 04       	lea    0x4(%rsp),%rbx # 0x400f0e 跳转到这里, %rsp+0x4 -> %rbx
  400f35:	48 8d 6c 24 18       	lea    0x18(%rsp),%rbp
  400f3a:	eb db                	jmp    400f17 <phase_2+0x1b>
  400f3c:	48 83 c4 28          	add    $0x28,%rsp # 出栈
  400f40:	5b                   	pop    %rbx # 出栈
  400f41:	5d                   	pop    %rbp # 出栈
  400f42:	c3                   	ret
  
000000000040145c <read_six_numbers>:
  40145c:	48 83 ec 18          	sub    $0x18,%rsp # 在栈中开辟一块 24 字节的内存空间
  401460:	48 89 f2             	mov    %rsi,%rdx # 把 %rdx 中存储的栈指针移到 %rsi
  401463:	48 8d 4e 04          	lea    0x4(%rsi),%rcx # *(%rsi + 0x4)
  401467:	48 8d 46 14          	lea    0x14(%rsi),%rax
  40146b:	48 89 44 24 08       	mov    %rax,0x8(%rsp)
  401470:	48 8d 46 10          	lea    0x10(%rsi),%rax
  401474:	48 89 04 24          	mov    %rax,(%rsp)
  401478:	4c 8d 4e 0c          	lea    0xc(%rsi),%r9
  40147c:	4c 8d 46 08          	lea    0x8(%rsi),%r8
  401480:	be c3 25 40 00       	mov    $,%esi
  401485:	b8 00 00 00 00       	mov    $0x0,%eax
  40148a:	e8 61 f7 ff ff       	call   400bf0 <__isoc99_sscanf@plt>
  40148f:	83 f8 05             	cmp    $0x5,%eax
  401492:	7f 05                	jg     401499 <read_six_numbers+0x3d>
  401494:	e8 a1 ff ff ff       	call   40143a <explode_bomb>
  401499:	48 83 c4 18          	add    $0x18,%rsp
  40149d:	c3                   	ret

~~~

`read_six_numbers` 函数从字符串中读取6个数字，放入一个 int* 中，根据函数中 sscanf 传入的字符串常量可知

![image-20230326143646541](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bed/image-20230326143646541.png)

根据`0x400f0a`处的 `cmp`，可知第一个数字是 1

接下来有一段类似循环结构的操作来判断剩下的5个数字，很容易就可以推出每个数字都是前面一个数字的两倍

拿到第二个密钥

> 1 2 4 8 16 32

## phase_3

~~~assembly
0000000000400f43 <phase_3>:
  400f43:	48 83 ec 18          	sub    $0x18,%rsp # 开辟一块0x18个字节的内存空间
  400f47:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx # 
  400f4c:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx
  400f51:	be cf 25 40 00       	mov    $0x4025cf,%esi
  400f56:	b8 00 00 00 00       	mov    $0x0,%eax
  400f5b:	e8 90 fc ff ff       	call   400bf0 <__isoc99_sscanf@plt>
  400f60:	83 f8 01             	cmp    $0x1,%eax
  400f63:	7f 05                	jg     400f6a <phase_3+0x27> # 0x1 < %eax 则进行跳转
  400f65:	e8 d0 04 00 00       	call   40143a <explode_bomb>
  400f6a:	83 7c 24 08 07       	cmpl   $0x7,0x8(%rsp)
  400f6f:	77 3c                	ja     400fad <phase_3+0x6a>
  400f71:	8b 44 24 08          	mov    0x8(%rsp),%eax
  400f75:	ff 24 c5 70 24 40 00 	jmp    *0x402470(,%rax,8)
  400f7c:	b8 cf 00 00 00       	mov    $0xcf,%eax
  400f81:	eb 3b                	jmp    400fbe <phase_3+0x7b>
  400f83:	b8 c3 02 00 00       	mov    $0x2c3,%eax
  400f88:	eb 34                	jmp    400fbe <phase_3+0x7b>
  400f8a:	b8 00 01 00 00       	mov    $0x100,%eax
  400f8f:	eb 2d                	jmp    400fbe <phase_3+0x7b>
  400f91:	b8 85 01 00 00       	mov    $0x185,%eax
  400f96:	eb 26                	jmp    400fbe <phase_3+0x7b>
  400f98:	b8 ce 00 00 00       	mov    $0xce,%eax
  400f9d:	eb 1f                	jmp    400fbe <phase_3+0x7b>
  400f9f:	b8 aa 02 00 00       	mov    $0x2aa,%eax
  400fa4:	eb 18                	jmp    400fbe <phase_3+0x7b>
  400fa6:	b8 47 01 00 00       	mov    $0x147,%eax
  400fab:	eb 11                	jmp    400fbe <phase_3+0x7b>
  400fad:	e8 88 04 00 00       	call   40143a <explode_bomb>
  400fb2:	b8 00 00 00 00       	mov    $0x0,%eax
  400fb7:	eb 05                	jmp    400fbe <phase_3+0x7b>
  400fb9:	b8 37 01 00 00       	mov    $0x137,%eax
  400fbe:	3b 44 24 0c          	cmp    0xc(%rsp),%eax
  400fc2:	74 05                	je     400fc9 <phase_3+0x86>
  400fc4:	e8 71 04 00 00       	call   40143a <explode_bomb>
  400fc9:	48 83 c4 18          	add    $0x18,%rsp
  400fcd:	c3                   	ret
~~~

一来就看到了一个字符串常量 `0x4025cf` ，看起来似乎是传入 sscanf 的 format，拿gdb看看

![image-20230326155410212](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bed/image-20230326155410212.png)

至少可以确定这次密钥的格式了。
