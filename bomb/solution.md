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
  400f47:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx
  400f4c:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx # 把rdx加载到 0x8(%rsp)
  400f51:	be cf 25 40 00       	mov    $0x4025cf,%esi
  400f56:	b8 00 00 00 00       	mov    $0x0,%eax
  400f5b:	e8 90 fc ff ff       	call   400bf0 <__isoc99_sscanf@plt>
  400f60:	83 f8 01             	cmp    $0x1,%eax
  400f63:	7f 05                	jg     400f6a <phase_3+0x27> # %eax 为1则进行跳转, 这里是如果输入符合格式则跳转，不跳转就会直接explode
  400f65:	e8 d0 04 00 00       	call   40143a <explode_bomb>
  400f6a:	83 7c 24 08 07       	cmpl   $0x7,0x8(%rsp) # 0x7 < 0x8(%rsp) 则进行跳转，直接explode, 这里得出x1不能大于7
  400f6f:	77 3c                	ja     400fad <phase_3+0x6a>
  400f71:	8b 44 24 08          	mov    0x8(%rsp),%eax
  400f75:	ff 24 c5 70 24 40 00 	jmp    *0x402470(,%rax,8) # 根据跳转表进行跳转
  0 > 400f7c:	b8 cf 00 00 00       	mov    $0xcf,%eax
  400f81:	eb 3b                	jmp    400fbe <phase_3+0x7b>
  2 > 400f83:	b8 c3 02 00 00       	mov    $0x2c3,%eax
  400f88:	eb 34                	jmp    400fbe <phase_3+0x7b>
  3 > 400f8a:	b8 00 01 00 00       	mov    $0x100,%eax
  400f8f:	eb 2d                	jmp    400fbe <phase_3+0x7b>
  4 > 400f91:	b8 85 01 00 00       	mov    $0x185,%eax
  400f96:	eb 26                	jmp    400fbe <phase_3+0x7b>
  5 > 400f98:	b8 ce 00 00 00       	mov    $0xce,%eax
  400f9d:	eb 1f                	jmp    400fbe <phase_3+0x7b>
  6 > 400f9f:	b8 aa 02 00 00       	mov    $0x2aa,%eax
  400fa4:	eb 18                	jmp    400fbe <phase_3+0x7b>
  7 > 400fa6:	b8 47 01 00 00       	mov    $0x147,%eax
  400fab:	eb 11                	jmp    400fbe <phase_3+0x7b>
  400fad:	e8 88 04 00 00       	call   40143a <explode_bomb>
  400fb2:	b8 00 00 00 00       	mov    $0x0,%eax
  400fb7:	eb 05                	jmp    400fbe <phase_3+0x7b>
  1 > 400fb9:	b8 37 01 00 00       	mov    $0x137,%eax
  400fbe:	3b 44 24 0c          	cmp    0xc(%rsp),%eax # x2与%eax比较 0 -> 0xcf 1 -> 0x137 2 -> 0x2c3 3 -> 0x100 4 -> 0x185 5 -> 0xce 6 -> 0x2aa 7 -> 0x147
  400fc2:	74 05                	je     400fc9 <phase_3+0x86>
  400fc4:	e8 71 04 00 00       	call   40143a <explode_bomb>
  400fc9:	48 83 c4 18          	add    $0x18,%rsp
  400fcd:	c3                   	ret
~~~

一来就看到了一个字符串常量 `0x4025cf` ，看起来似乎是传入 sscanf 的 format，拿gdb看看

![image-20230326155410212](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bed/image-20230326155410212.png)

至少可以确定这次密钥的格式了。

> jmp    *0x402470(,%rax,8) # 根据跳转表进行跳转

这说明有一个switch语句，看看跳转表

![image-20230410001702256](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bed/image-20230410001702256.png)

那么可以得出

> 0 0xcf
>
> 1 0x137
>
> 2 0x2c3
>
> 3 0x100
>
> 4 0x185
>
> 5 0xce
>
> 6 0x2aa
>
> 7 0x147

以上任一即可

## phase_4

phase_4

~~~assembly
   0x000000000040100c <+0>:	sub    $0x18,%rsp
   0x0000000000401010 <+4>:	lea    0xc(%rsp),%rcx
   0x0000000000401015 <+9>:	lea    0x8(%rsp),%rdx
   0x000000000040101a <+14>:	mov    $0x4025cf,%esi
   0x000000000040101f <+19>:	mov    $0x0,%eax
   0x0000000000401024 <+24>:	call   0x400bf0 <__isoc99_sscanf@plt>
   0x0000000000401029 <+29>:	cmp    $0x2,%eax
   0x000000000040102c <+32>:	jne    0x401035 <phase_4+41> # 如果sscanf没扫到两个数字就explode
   0x000000000040102e <+34>:	cmpl   $0xe,0x8(%rsp) # 0xe = 14
   0x0000000000401033 <+39>:	jbe    0x40103a <phase_4+46> # 小于等于14时跳转，否则explode
   0x0000000000401035 <+41>:	call   0x40143a <explode_bomb>
   0x000000000040103a <+46>:	mov    $0xe,%edx # 14
   0x000000000040103f <+51>:	mov    $0x0,%esi # 0
   0x0000000000401044 <+56>:	mov    0x8(%rsp),%edi # 第一个数作为参数传入func4
   0x0000000000401048 <+60>:	call   0x400fce <func4>
   0x000000000040104d <+65>:	test   %eax,%eax
   0x000000000040104f <+67>:	jne    0x401058 <phase_4+76> # 返回值不为0时跳转，explode
   0x0000000000401051 <+69>:	cmpl   $0x0,0xc(%rsp) # 第二个数等于0
   0x0000000000401056 <+74>:	je     0x40105d <phase_4+81>
   0x0000000000401058 <+76>:	call   0x40143a <explode_bomb>
   0x000000000040105d <+81>:	add    $0x18,%rsp
   0x0000000000401061 <+85>:	ret
~~~

func4

~~~assembly
   0x0000000000400fce <+0>:	sub    $0x8,%rsp
   0x0000000000400fd2 <+4>:	mov    %edx,%eax # eax = edx
   0x0000000000400fd4 <+6>:	sub    %esi,%eax # eax -= esi
   0x0000000000400fd6 <+8>:	mov    %eax,%ecx # ecx = eax
   0x0000000000400fd8 <+10>:	shr    $0x1f,%ecx # ecx >> 31 逻辑右移拿到符号位
   0x0000000000400fdb <+13>:	add    %ecx,%eax # eax += ecx
   0x0000000000400fdd <+15>:	sar    %eax # 算数右移一位，即 eax /= 2
   0x0000000000400fdf <+17>:	lea    (%rax,%rsi,1),%ecx # ecx = eax + esi
   0x0000000000400fe2 <+20>:	cmp    %edi,%ecx
   0x0000000000400fe4 <+22>:	jle    0x400ff2 <func4+36> # ecx < edi 时跳转
   0x0000000000400fe6 <+24>:	lea    -0x1(%rcx),%edx
   0x0000000000400fe9 <+27>:	call   0x400fce <func4>
   0x0000000000400fee <+32>:	add    %eax,%eax
   0x0000000000400ff0 <+34>:	jmp    0x401007 <func4+57>
   0x0000000000400ff2 <+36>:	mov    $0x0,%eax # 设置返回值为0
   0x0000000000400ff7 <+41>:	cmp    %edi,%ecx
   0x0000000000400ff9 <+43>:	jge    0x401007 <func4+57> # edi >= ecx 时跳转
   0x0000000000400ffb <+45>:	lea    0x1(%rcx),%esi
   0x0000000000400ffe <+48>:	call   0x400fce <func4>
   0x0000000000401003 <+53>:	lea    0x1(%rax,%rax,1),%eax
   0x0000000000401007 <+57>:	add    $0x8,%rsp
   0x000000000040100b <+61>:	ret
~~~

![image-20230326155410212](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bed/image-20230326155410212.png)

拿到格式字符串

将func4转译为c代码

~~~c
int func4(int edi, int esi, int edx) {
    int eax = edx - esi;
    int ecx = eax >> 31;
    eax += ecx;
    eax = eax >> 1;
    ecx = eax + esi;
    if (ecx <= edi) {
        eax = 0;
        if (edi >= ecx) {
            return 0;
        }
        esi = ecx + 1;
        eax = func4(edi, esi, edx);
        eax = 2 * eax + 1;
    } else {
        edx = ecx - 1;
        eax = func4(edi, esi, edx);
        eax += eax;
    }
    return eax;
}
~~~

简化一下

~~~c
int func4(int x, int a, int b)
{
    int num = b - a;
    num = (num + num >> 31) / 2;
    int c = num + a;
    if (c <= x) {
    	if (c >= x) return 0;
        return 2 * func4(x, num+1, b) + 1;
    }
    return 2 * func4(x, a, num-1);
}
~~~

实际上就是二分法在区间范围内找数，找到了就返回0。

> 7 0

## phase_5

~~~assembly
   0x0000000000401062 <+0>:	push   %rbx
   0x0000000000401063 <+1>:	sub    $0x20,%rsp
   0x0000000000401067 <+5>:	mov    %rdi,%rbx # rbx = rdi
   0x000000000040106a <+8>:	mov    %fs:0x28,%rax
   0x0000000000401073 <+17>:	mov    %rax,0x18(%rsp)
   0x0000000000401078 <+22>:	xor    %eax,%eax
   0x000000000040107a <+24>:	call   0x40131b <string_length>
   0x000000000040107f <+29>:	cmp    $0x6,%eax # 这里可以推出字符串长度为6
   0x0000000000401082 <+32>:	je     0x4010d2 <phase_5+112>
   0x0000000000401084 <+34>:	call   0x40143a <explode_bomb>
   0x0000000000401089 <+39>:	jmp    0x4010d2 <phase_5+112>
   0x000000000040108b <+41>:	movzbl (%rbx,%rax,1),%ecx
   0x000000000040108f <+45>:	mov    %cl,(%rsp)
   0x0000000000401092 <+48>:	mov    (%rsp),%rdx
   0x0000000000401096 <+52>:	and    $0xf,%edx
   0x0000000000401099 <+55>:	movzbl 0x4024b0(%rdx),%edx
   0x00000000004010a0 <+62>:	mov    %dl,0x10(%rsp,%rax,1)
   0x00000000004010a4 <+66>:	add    $0x1,%rax
   0x00000000004010a8 <+70>:	cmp    $0x6,%rax
   0x00000000004010ac <+74>:	jne    0x40108b <phase_5+41>
   0x00000000004010ae <+76>:	movb   $0x0,0x16(%rsp)
   0x00000000004010b3 <+81>:	mov    $0x40245e,%esi
   0x00000000004010b8 <+86>:	lea    0x10(%rsp),%rdi
   0x00000000004010bd <+91>:	call   0x401338 <strings_not_equal>
   0x00000000004010c2 <+96>:	test   %eax,%eax
   0x00000000004010c4 <+98>:	je     0x4010d9 <phase_5+119>
   0x00000000004010c6 <+100>:	call   0x40143a <explode_bomb>
   0x00000000004010cb <+105>:	nopl   0x0(%rax,%rax,1)
   0x00000000004010d0 <+110>:	jmp    0x4010d9 <phase_5+119>
   0x00000000004010d2 <+112>:	mov    $0x0,%eax
   0x00000000004010d7 <+117>:	jmp    0x40108b <phase_5+41>
   0x00000000004010d9 <+119>:	mov    0x18(%rsp),%rax
   0x00000000004010de <+124>:	xor    %fs:0x28,%rax
   0x00000000004010e7 <+133>:	je     0x4010ee <phase_5+140>
   0x00000000004010e9 <+135>:	call   0x400b30 <__stack_chk_fail@plt>
   0x00000000004010ee <+140>:	add    $0x20,%rsp
   0x00000000004010f2 <+144>:	pop    %rbx
   0x00000000004010f3 <+145>:	ret
~~~

总之先看下 +74 处的字符串常量

![image-20230410121134474](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bed/image-20230410121134474.png)

再看看 +55 处的字符串常量

![image-20230410121720641](https://persecution-1301196908.cos.ap-chongqing.myqcloud.com/image_bed/image-20230410121720641.png)

根据汇编代码的意思是，把输入字符的后四位作为索引在这个字符串中找字符，拼接成一个字符串，然后与flyers比较。

逆推可得:

> YONEFw

## phase_6

~~~assembly
   0x00000000004010f4 <+0>:	push   %r14
   0x00000000004010f6 <+2>:	push   %r13
   0x00000000004010f8 <+4>:	push   %r12
   0x00000000004010fa <+6>:	push   %rbp
   0x00000000004010fb <+7>:	push   %rbx
   0x00000000004010fc <+8>:	sub    $0x50,%rsp
   0x0000000000401100 <+12>:	mov    %rsp,%r13 # r13 = rsp
   0x0000000000401103 <+15>:	mov    %rsp,%rsi # 将开辟的空间传入read_six_numbers
   0x0000000000401106 <+18>:	call   0x40145c <read_six_numbers> # 读6个数 前面phase2遇到过
   0x000000000040110b <+23>:	mov    %rsp,%r14 # r14 = rsp
   0x000000000040110e <+26>:	mov    $0x0,%r12d # r12d = 0
   0x0000000000401114 <+32>:	mov    %r13,%rbp # rbp = r13
   0x0000000000401117 <+35>:	mov    0x0(%r13),%eax # eax = 数组的第一个元素
   0x000000000040111b <+39>:	sub    $0x1,%eax # eax -= 1
   0x000000000040111e <+42>:	cmp    $0x5,%eax # eax <= 5 跳转，不跳转就explode
   0x0000000000401121 <+45>:	jbe    0x401128 <phase_6+52>
   0x0000000000401123 <+47>:	call   0x40143a <explode_bomb>
   0x0000000000401128 <+52>:	add    $0x1,%r12d # r12d += 1
   0x000000000040112c <+56>:	cmp    $0x6,%r12d 
   0x0000000000401130 <+60>:	je     0x401153 <phase_6+95> # r12d == 6 则跳转
   0x0000000000401132 <+62>:	mov    %r12d,%ebx # ebx = r12d
   0x0000000000401135 <+65>:	movslq %ebx,%rax # rax = ebx
   0x0000000000401138 <+68>:	mov    (%rsp,%rax,4),%eax # eax = *(rsp + 4 * rax) (这段是在拿数组里的元素)
   0x000000000040113b <+71>:	cmp    %eax,0x0(%rbp) # eax != 数组第一个元素
   0x000000000040113e <+74>:	jne    0x401145 <phase_6+81>
   0x0000000000401140 <+76>:	call   0x40143a <explode_bomb>
   0x0000000000401145 <+81>:	add    $0x1,%ebx # ebx += 1
   0x0000000000401148 <+84>:	cmp    $0x5,%ebx # ebx <= 5 则跳转
   0x000000000040114b <+87>:	jle    0x401135 <phase_6+65> # 跳转回去了，这里是一个类似while循环的结构
   0x000000000040114d <+89>:	add    $0x4,%r13
   0x0000000000401151 <+93>:	jmp    0x401114 <phase_6+32>
   0x0000000000401153 <+95>:	lea    0x18(%rsp),%rsi
   0x0000000000401158 <+100>:	mov    %r14,%rax
   0x000000000040115b <+103>:	mov    $0x7,%ecx
   0x0000000000401160 <+108>:	mov    %ecx,%edx
   0x0000000000401162 <+110>:	sub    (%rax),%edx
   0x0000000000401164 <+112>:	mov    %edx,(%rax)
   0x0000000000401166 <+114>:	add    $0x4,%rax
   0x000000000040116a <+118>:	cmp    %rsi,%rax
   0x000000000040116d <+121>:	jne    0x401160 <phase_6+108>
   0x000000000040116f <+123>:	mov    $0x0,%esi
   0x0000000000401174 <+128>:	jmp    0x401197 <phase_6+163>
   0x0000000000401176 <+130>:	mov    0x8(%rdx),%rdx
   0x000000000040117a <+134>:	add    $0x1,%eax
   0x000000000040117d <+137>:	cmp    %ecx,%eax
   0x000000000040117f <+139>:	jne    0x401176 <phase_6+130>
   0x0000000000401181 <+141>:	jmp    0x401188 <phase_6+148>
   0x0000000000401183 <+143>:	mov    $0x6032d0,%edx
   0x0000000000401188 <+148>:	mov    %rdx,0x20(%rsp,%rsi,2)
   0x000000000040118d <+153>:	add    $0x4,%rsi
   0x0000000000401191 <+157>:	cmp    $0x18,%rsi
   0x0000000000401195 <+161>:	je     0x4011ab <phase_6+183>
   0x0000000000401197 <+163>:	mov    (%rsp,%rsi,1),%ecx
   0x000000000040119a <+166>:	cmp    $0x1,%ecx
   0x000000000040119d <+169>:	jle    0x401183 <phase_6+143>
   0x000000000040119f <+171>:	mov    $0x1,%eax
   0x00000000004011a4 <+176>:	mov    $0x6032d0,%edx
   0x00000000004011a9 <+181>:	jmp    0x401176 <phase_6+130>
   0x00000000004011ab <+183>:	mov    0x20(%rsp),%rbx
   0x00000000004011b0 <+188>:	lea    0x28(%rsp),%rax
   0x00000000004011b5 <+193>:	lea    0x50(%rsp),%rsi
   0x00000000004011ba <+198>:	mov    %rbx,%rcx
   0x00000000004011bd <+201>:	mov    (%rax),%rdx
   0x00000000004011c0 <+204>:	mov    %rdx,0x8(%rcx)
   0x00000000004011c4 <+208>:	add    $0x8,%rax
   0x00000000004011c8 <+212>:	cmp    %rsi,%rax
   0x00000000004011cb <+215>:	je     0x4011d2 <phase_6+222>
   0x00000000004011cd <+217>:	mov    %rdx,%rcx
   0x00000000004011d0 <+220>:	jmp    0x4011bd <phase_6+201>
   0x00000000004011d2 <+222>:	movq   $0x0,0x8(%rdx)
   0x00000000004011da <+230>:	mov    $0x5,%ebp
   0x00000000004011df <+235>:	mov    0x8(%rbx),%rax
   0x00000000004011e3 <+239>:	mov    (%rax),%eax
   0x00000000004011e5 <+241>:	cmp    %eax,(%rbx)
   0x00000000004011e7 <+243>:	jge    0x4011ee <phase_6+250>
   0x00000000004011e9 <+245>:	call   0x40143a <explode_bomb>
   0x00000000004011ee <+250>:	mov    0x8(%rbx),%rbx
   0x00000000004011f2 <+254>:	sub    $0x1,%ebp
   0x00000000004011f5 <+257>:	jne    0x4011df <phase_6+235>
   0x00000000004011f7 <+259>:	add    $0x50,%rsp
   0x00000000004011fb <+263>:	pop    %rbx
   0x00000000004011fc <+264>:	pop    %rbp
   0x00000000004011fd <+265>:	pop    %r12
   0x00000000004011ff <+267>:	pop    %r13
   0x0000000000401201 <+269>:	pop    %r14
   0x0000000000401203 <+271>:	ret
~~~

转译为c代码

~~~c
typedef struct {
    int val;
    ListNode* next;
} ListNode;

void phase_6(char* output)
{
    int array[6];
    ListNode* node_array[6];
    read_six_numbers(output, array);
    // 数字范围必须为1-6且互不重复
    for (int i = 0; i != 6; i++) {
        int num = array[i];
        num--;
        if ((unsigned int)num > 5)		// 最大为6
            explode_bomb();
        for (int j = i+1; j <= 5; j++) {
            if (array[i] == array[j])	// 每个元素都不重复
                explode_bomb();
        }
    }
    // 修改 array
	for (int i = 0; i < 6; i ++) {
        array[i] = (7 - array[i]);
	}
    // 生成 node_array
	for (int i = 0; i < 6; i ++) {
        int cur = array[i];
        ListNode* node = 0x6032d0;		// 链表head
        if (cur > 1) {
            for (int j = 1; j < cur; j++) {
                node = node->next;
            }
        }
        node_array[i] = node;
	}
    for (int i = 0; i < 5; i++) {
        node_array[i]->next = node_array[i+1];
    }
    //0x6032d0 0x6032e0 0x6032f0 0x603300 0x603310 0x603320
    //332 168 924 691 477 443
    // 6 5 4 3 2 1
    // 5 6 1 2 3 4 -> 4 3 2 1 6 5
    ListNode* ptr = node_array[0];
    for (int i = 5; i > 0; i--) {
        if (ptr->val < ptr->next->val)
            explode_bomb();
        ptr = ptr->next;
    }
}
~~~

> 答案是 4 3 2 1 6 5

好像还有个隐藏phase，不想做了...
