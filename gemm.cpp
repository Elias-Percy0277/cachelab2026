/*
请注意，你的代码不能出现任何 int/short/char/float/double/auto 等局部变量/函数传参，我们仅允许使用 reg 定义的寄存器变量。
其中 reg 等价于一个 int。

你不能自己申请额外的内存，即不能使用 new/malloc，作为补偿我们传入了一段 buffer，大小为 BUFFER_SIZE = 64，你可以视情况使用。

我们的数组按照 A, B, C, buffer 的顺序在内存上连续紧密排列，且 &A = 0x30000000（这是模拟的设定，不是 A 的真实地址）

如果你需要以更自由的方式访问内存，你可以以相对 A 的方式访问，比如 A[100]，用 *(0x30000000) 是无法访问到的。

如果你有定义常量的需求（更严谨的说法是，你想定义的是汇编层面的立即数，不应该占用寄存器），请参考下面这种方式使用宏定义来完成。
*/

#include "cachelab.h"

#define m case0_m
#define n case0_n
#define p case0_p

// 我们用这个 2*2*2 的矩阵乘法来演示寄存器是怎么被分配的
void gemm_case0(ptr_reg A, ptr_reg B, ptr_reg C, ptr_reg buffer)
{ // allocate 0 1 2 3
  for (reg i = 0; i < m; ++i)
  { // allocate 4
    for (reg j = 0; j < p; ++j)
    {               // allocate 5
      reg tmpc = 0; // allocate 6
      for (reg k = 0; k < n; ++k)
      {                          // allocate 7
        reg tmpa = A[i * n + k]; // allocate 8
        reg tmpb = B[k * p + j]; // allocate 9
        tmpc += tmpa * tmpb;
      } // free 9 8
      // free 7
      C[i * p + j] = tmpc;
    } // free 6
    // free 5
  }
  // free 4
} // free 3 2 1 0

#undef m
#undef n
#undef p

#define m case1_m   // 16
#define n case1_n   // 16
#define p case1_p   // 16

void gemm_case1(ptr_reg A, ptr_reg B, ptr_reg C, ptr_reg buffer)
{
  for (reg i0(0); i0 < m; i0 += 4)
  {
    for (reg j0(0); j0 < p; j0 += 4)
    {
      reg acc[4][4];                     
      for (reg ii(0); ii < 4; ++ii)
        for (reg jj(0); jj < 4; ++jj)
          acc[ii][jj] = 0;

      for (reg k(0); k < n; ++k)
      {
        reg a0(A[(i0 + 0) * n + k]);     
        reg a1(A[(i0 + 1) * n + k]);
        reg a2(A[(i0 + 2) * n + k]);
        reg a3(A[(i0 + 3) * n + k]);

        for (reg jj(0); jj < 4; ++jj)
        {
          reg b(B[k * p + j0 + jj]);     
          acc[0][jj] += a0 * b;
          acc[1][jj] += a1 * b;
          acc[2][jj] += a2 * b;
          acc[3][jj] += a3 * b;
        }
      }

      for (reg ii(0); ii < 4; ++ii)
        for (reg jj(0); jj < 4; ++jj)
          C[(i0 + ii) * p + j0 + jj] = acc[ii][jj];
    }
  }
}

#undef m
#undef n
#undef p

#define m case2_m
#define n case2_n
#define p case2_p
#define BI 4        
#define BJ 4 

void gemm_case2(ptr_reg A, ptr_reg B, ptr_reg C, ptr_reg buffer)
{
  for (reg i0(0); i0 < m; i0 += BI)
  {
    for (reg j0(0); j0 < p; j0 += BJ)
    {
      reg acc[BI][BJ];
      for (reg ii(0); ii < BI; ++ii)
        for (reg jj(0); jj < BJ; ++jj)
          acc[ii][jj] = 0;

      for (reg k(0); k < n; ++k)
      {
        reg a[BI];
        for (reg ii(0); ii < BI; ++ii)     
          a[ii] = 0;
        for (reg ii(0); ii < BI; ++ii)
          if (i0 + ii < m)                
            a[ii] = A[(i0 + ii) * n + k];

        for (reg jj(0); jj < BJ; ++jj)
          if (j0 + jj < p)                 
          {
            reg b(B[k * p + j0 + jj]);
            for (reg ii(0); ii < BI; ++ii)
              acc[ii][jj] += a[ii] * b;
          }
      }

      for (reg ii(0); ii < BI; ++ii)
        for (reg jj(0); jj < BJ; ++jj)
          if (i0 + ii < m && j0 + jj < p)  
            C[(i0 + ii) * p + j0 + jj] = acc[ii][jj];
    }
  }
}

#undef m
#undef n
#undef p

#define m case3_m
#define n case3_n
#define p case3_p

void gemm_case3(ptr_reg A, ptr_reg B, ptr_reg C, ptr_reg buffer)
{
  for (reg i0(0); i0 < m; i0 += 4)
  {
    for (reg j0(0); j0 < p; j0 += 6)
    {
      reg acc[4][6];
      for (reg ii(0); ii < 4; ++ii)
        for (reg jj(0); jj < 6; ++jj)
          acc[ii][jj] = 0;

      for (reg k(0); k < n; ++k)
      {
        reg a0(0), a1(0), a2(0), a3(0);
        if (i0 + 0 < m) a0 = A[(i0 + 0) * n + k];
        if (i0 + 1 < m) a1 = A[(i0 + 1) * n + k];
        if (i0 + 2 < m) a2 = A[(i0 + 2) * n + k];
        if (i0 + 3 < m) a3 = A[(i0 + 3) * n + k];

        if (j0 + 0 < p)
        {
          reg b0(B[k * p + j0 + 0]);
          acc[0][0] += a0 * b0;
          acc[1][0] += a1 * b0;
          acc[2][0] += a2 * b0;
          acc[3][0] += a3 * b0;
        }
        if (j0 + 1 < p)
        {
          reg b1(B[k * p + j0 + 1]);
          acc[0][1] += a0 * b1;
          acc[1][1] += a1 * b1;
          acc[2][1] += a2 * b1;
          acc[3][1] += a3 * b1;
        }
        if (j0 + 2 < p)
        {
          reg b2(B[k * p + j0 + 2]);
          acc[0][2] += a0 * b2;
          acc[1][2] += a1 * b2;
          acc[2][2] += a2 * b2;
          acc[3][2] += a3 * b2;
        }
        if (j0 + 3 < p)
        {
          reg b3(B[k * p + j0 + 3]);
          acc[0][3] += a0 * b3;
          acc[1][3] += a1 * b3;
          acc[2][3] += a2 * b3;
          acc[3][3] += a3 * b3;
        }
        if (j0 + 4 < p)
        {
          reg b4(B[k * p + j0 + 4]);
          acc[0][4] += a0 * b4;
          acc[1][4] += a1 * b4;
          acc[2][4] += a2 * b4;
          acc[3][4] += a3 * b4;
        }
        if (j0 + 5 < p)
        {
          reg b5(B[k * p + j0 + 5]);
          acc[0][5] += a0 * b5;
          acc[1][5] += a1 * b5;
          acc[2][5] += a2 * b5;
          acc[3][5] += a3 * b5;
        }
      }

      for (reg ii(0); ii < 4; ++ii)
        for (reg jj(0); jj < 6; ++jj)
          if (i0 + ii < m && j0 + jj < p)
            C[(i0 + ii) * p + j0 + jj] = acc[ii][jj];
    }
  }
}

#undef m
#undef n
#undef p
