/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    // 32x32
    if (M == 32) {
        int i, j, m;
        int a, b, c, d, e, f, g, h;
        for (i = 0; i < M; i += 8)
            for (j = 0; j < N; j += 8)
                for (m = i; m < i + 8; m++) {
                    a = A[m][0 + j];
                    b = A[m][1 + j];
                    c = A[m][2 + j];
                    d = A[m][3 + j];
                    e = A[m][4 + j];
                    f = A[m][5 + j];
                    g = A[m][6 + j];
                    h = A[m][7 + j];
                    B[j][m] = a;
                    B[1 + j][m] = b;
                    B[2 + j][m] = c;
                    B[3 + j][m] = d;
                    B[4 + j][m] = e;
                    B[5 + j][m] = f;
                    B[6 + j][m] = g;
                    B[7 + j][m] = h;
                }
    }
    // 64x64
    if (M == 64) {
        int i, j, x, y;
		int x1, x2, x3, x4, x5, x6, x7, x8;
		for (i = 0; i < N; i += 8)
			for (j = 0; j < M; j += 8)
			{
				for (x = i; x < i + 4; ++x)
				{
					x1 = A[x][j]; x2 = A[x][j+1]; x3 = A[x][j+2]; x4 = A[x][j+3];
					x5 = A[x][j+4]; x6 = A[x][j+5]; x7 = A[x][j+6]; x8 = A[x][j+7];
					
					B[j][x] = x1; B[j+1][x] = x2; B[j+2][x] = x3; B[j+3][x] = x4;
					B[j][x+4] = x5; B[j+1][x+4] = x6; B[j+2][x+4] = x7; B[j+3][x+4] = x8;
				}
				for (y = j; y < j + 4; ++y)
				{
					x1 = A[i+4][y]; x2 = A[i+5][y]; x3 = A[i+6][y]; x4 = A[i+7][y];
					x5 = B[y][i+4]; x6 = B[y][i+5]; x7 = B[y][i+6]; x8 = B[y][i+7];
					
					B[y][i+4] = x1; B[y][i+5] = x2; B[y][i+6] = x3; B[y][i+7] = x4;
					B[y+4][i] = x5; B[y+4][i+1] = x6; B[y+4][i+2] = x7; B[y+4][i+3] = x8;
				}
				for (x = i + 4; x < i + 8; ++x)
				{
					x1 = A[x][j+4]; x2 = A[x][j+5]; x3 = A[x][j+6]; x4 = A[x][j+7];
					B[j+4][x] = x1; B[j+5][x] = x2; B[j+6][x] = x3; B[j+7][x] = x4;
				}
			}
    }
    // 61x67
    if (M == 61) {
        // 还是8分块，多出来的地方补上
        // 跟32x差不多
		int i, j, v1, v2, v3, v4, v5, v6, v7, v8;
		int n = N / 8 * 8;
		int m = M / 8 * 8;
		for (j = 0; j < m; j += 8)
			for (i = 0; i < n; ++i)
			{
				v1 = A[i][j];
				v2 = A[i][j+1];
				v3 = A[i][j+2];
				v4 = A[i][j+3];
				v5 = A[i][j+4];
				v6 = A[i][j+5];
				v7 = A[i][j+6];
				v8 = A[i][j+7];
				
				B[j][i] = v1;
				B[j+1][i] = v2;
				B[j+2][i] = v3;
				B[j+3][i] = v4;
				B[j+4][i] = v5;
				B[j+5][i] = v6;
				B[j+6][i] = v7;
				B[j+7][i] = v8;
			}
		for (i = n; i < N; ++i)
			for (j = m; j < M; ++j)
			{
				v1 = A[i][j];
				B[j][i] = v1;
			}
		for (i = 0; i < N; ++i)
			for (j = m; j < M; ++j)
			{
				v1 = A[i][j];
				B[j][i] = v1;
			}
		for (i = n; i < N; ++i)
			for (j = 0; j < M; ++j)
			{
				v1 = A[i][j];
				B[j][i] = v1;
			}
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

