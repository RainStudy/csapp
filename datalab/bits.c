/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  // 分别取反后进行与运算得到 -> 两个数字对应位上的数均为0时
  // 得到结果的对应位上的数字为1，其他情况下为0
  // 不取反进行与运算得到 -> 两个数字对应位上的数均为1时
  // 得到结果的对应位上的数字为1，其他情况下为0
  // 对这两个数分别取反后进行与运算 可得原本对应位数字不相同的位为1，其余为0
  return ~(~x & ~y) & ~(x & y);
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {

  return 0x1 << 31;

}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  int tmin = x + 1;
  x += tmin;
  x = ~x;
  x += !tmin; // exclude 0xffffffff
  return !x;
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  int mask = 0xAA + (0xAA << 8);
  mask += mask << 16;
  return !((mask & x) ^ mask);
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return ~x + 1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  // Tmin
  int sign = 0x1 << 31;
  int upperBound = ~(sign|0x39);
  int lowerBound = ~0x30;
  // 加上一个x，如果 > 0x39 则会溢出 符号位变为 1
  // 右移31位拿到符号位 若符号位为 1 则说明超出范围
  upperBound = sign & (upperBound + x) >> 31;
  // 加上一个 x 再加上 1，如果 > 0x30 则会溢出 符号位变为 0
  // 右移31位拿到符号位 若符号位为 1 则说明超出范围
  lowerBound = sign & (lowerBound + 1 + x) >> 31;

  return !(upperBound|lowerBound);
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  x = !!x;
  // 若为真，则不会溢出 值为 -1 0xffffffff
  // 若为假，则溢出 值为 0 0x00000000
  x = ~x + 1;
  // 为真时 与一下y可以得到y本身 取反与一下z会得到 0x00000000
  // 为假时 与一下y可以得到 0x0000000 取反与一下z会得到z本身
  return (x & y) | (~x & z);
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  int minusx = ~x + 1; // 得到-x
  int result = y + minusx; // 得到y - x
  int sign = (result >> 31) & 1; // 判断result的符号，如果y>=x，则sign等于0，否则等于1
  int xsign = (x >> 31) & 1; // 取出x的符号
  int ysign = (y >> 31) & 1; // 取出y的符号
  int bitXor = xsign ^ ysign; // 判断x和y符号是否一致
  return ((!bitXor) & (!sign)) | (bitXor & xsign); // 要么x和y符号相同并且x<=y，要么x和y符号不同并且x<0
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
  return ((x | (~x + 1)) >> 31) + 1;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
  int b16, b8, b4, b2, b1, b0;
  // 取符号位
  int sign = x >> 31;
  // 为正则不变 为负则取反
  x = (sign & ~x) | (~sign & x);
  // 高16位是否有1，有就右移16位
  b16 = !!(x >> 16) << 4;
  x = x >> b16;
  // 剩余的高8位是否有1，有就右移8位
  b8 = !!(x >> 8) << 3;
  x = x >> b8;
  // 剩余的高4位是否有1，有就右移4位
  b4 = !!(x >> 4) << 2;
  x = x >> b4;
  b2 = !!(x >> 2) << 1;
  x = x >> b2;
  b1 = !!(x >> 1);
  x = x >> b1;
  b0 = x;
  // 加上符号位
  return b16 + b8 + b4 + b2 + b1 + b0 + 1;
}
//float
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
  int exp = (uf&0x7F800000) >> 23; // 取出阶码
  int sign = uf&(1 << 31); // 取符号位
  if (exp == 0) return uf<<1|sign; // 若为非规格数，直接给uf乘以2后加上符号位即可
  if (exp == 255) return uf; // 若为无穷大或者NaN，直接返回自身
  exp = exp + 1; // 若uf乘以2（也就是阶码加1）后变成255，则返回无穷大
  if (exp == 255) return (0x7F800000|sign);
  return (exp << 23)|(uf&0x807FFFFF); // 返回阶码加1后的原符号数
}
/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
  int exp = ((uf&0x7F800000) >> 23) - 127; // 计算出指数
  int sign = uf >> 31; // 取符号位
  int frac = ((uf&0x007FFFFF) | 0x00800000); 
  if (!(uf&0x7FFFFFFF)) return 0; // 若原浮点数为0，则返回0

  if (exp > 31) return 0x80000000; // 若原浮点数指数大于31，返回溢出值
  if (exp < 0) return 0; // 若浮点数小于0，则返回0；

  if (exp > 23) frac = frac << (exp - 23); // 将小数转化为整数
  else frac = frac >> (23 - exp);

  if (!((frac >> 31) ^ sign)) return frac; // 判断是否溢出，若符号位没有变化，则没有溢出，返回正确的值
  else if (frac >> 31) return 0x80000000; // 原数为正值，现在为负值，返回溢出值
  else return ~frac + 1; // 原数为负值，现在为正值，返回相反数
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {
  int inf = 0xFF << 23; // 设定一个最大值，也就是阶码位置都为1
  int exp = x + 127; // 计算阶码
  if (exp <= 0) return 0; // 阶码小于等于0，则返回0
  if (exp >= 255) return inf; // 阶码大于等于255，则返回 inf
  return exp << 23;
}
