/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};


/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/// 以下是我自己定义的宏

// 是否使用 next fit 策略
#define NEXT_FIT 1

#define MAX(x, y) ((x) > (y)? (x) : (y))

/* 头部/脚部的大小 */
#define WSIZE 4
/* 双字 */
#define DSIZE 8

/* 扩展堆时的默认大小 */
#define CHUNKSIZE (1 << 12)

/* 设置头部和脚部的值, 块大小+分配位 */
#define PACK(size, alloc) ((size) | (alloc))

/* 读写指针p指向的值 */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) ((*(unsigned int *)(p)) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
// 给定块指针 计算它的 header 和 footer 的地址
#define HDRP(bp)       ((char *)(bp) - WSIZE)
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
// 给定块指针，计算前一个块和后一个块的地址
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* Function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void printblock(void *bp);
static void checkheap(int verbose);
static void checkblock(void *bp);

/* Global variables */
static char *heap_listp = 0;  /* Pointer to first block */
#ifdef NEXT_FIT
static char *rover;           /* Next fit rover */
#endif

// 拓展堆
void *extend_heap(size_t words) {
    // bp 总指向有效载荷
    char *bp;
    size_t size;
    // 按奇偶数考虑对齐
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    // 分配一块对应大小的内存
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;
    PUT(HDRP(bp), PACK(size, 0)); // 释放 header
    PUT(FTRP(bp), PACK(size, 0)); // 释放 footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // 分配新的 header
    // 判断周边块是否是空闲块，进行合并
    return coalesce(bp);
}

// 合并相邻的空闲内存块。根据前后相邻内存块的分配状态，有四种合并情况
void *coalesce(void *bp) {
    // 前一个块是否已经分配
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    // 后一个块是否已经分配
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    // 如果前后都是已经分配过的块，那就直接返回，不需要进行合并操作
    if (prev_alloc && next_alloc) {
        return bp;
    }
    // 如果前面分配过，后面一个块是空闲块，跟后面那个块合并
    else if (prev_alloc && !next_alloc) {
        // 把后面一个块的大小加上去
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        // 把新的大小写入
        // 先写头部 然后根据头部的大小定义尾部
        // tnnd 这宏差点给我绕晕了
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    // 如果前面是 free block，后面是 allocated block
    else if (!prev_alloc && next_alloc) {
        // 把前一个块的大小加上去
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        // 写新的大小写入
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(PREV_BLKP(bp)), PACK(size, 0));
        // bp自然要改成前一个
        bp = PREV_BLKP(bp);
    }
    // 如果前后都是 free block
    else {
        // 前后都加上去
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        // 把新的大小写入
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        // bp 自然要改成前一个
        bp = PREV_BLKP(bp);
    }
#ifdef NEXT_FIT
    // next fit 策略下，需要保证 recover 没有指向一个正在合并的空闲块
    // 指向合并后的
    if ((rover > (char *)bp) && (rover < NEXT_BLKP(bp)))
        rover = bp;
#endif
    return bp;
}

void *find_fit(size_t asize) {
#ifdef NEXT_FIT
    // next fit 策略下的实现
    // 记录下旧的 rover
    char *oldrover = rover;

    /* Search from the rover to the end of list */
    // 从当前位置开始搜索 直到链表的尾部
    for ( ; GET_SIZE(HDRP(rover)) > 0; rover = NEXT_BLKP(rover))
        if (!GET_ALLOC(HDRP(rover)) && (asize <= GET_SIZE(HDRP(rover))))
            return rover;

    /* search from start of list to old rover */
    // 从链表的开始搜索 直到 oldrover 的位置
    for (rover = heap_listp; rover < oldrover; rover = NEXT_BLKP(rover))
        if (!GET_ALLOC(HDRP(rover)) && (asize <= GET_SIZE(HDRP(rover))))
            return rover;

    // 没搜到，返回 NULL
    return NULL;  /* no fit found */
#else
    /* $begin mmfirstfit */
    /* First-fit search */
    // first fit 策略
    void *bp;

    // 从头搜到尾
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
            return bp;
        }
    }
    return NULL; /* No fit */
#endif
}

// place() 函数用于将指定大小的内存块放置在空闲内存块的起始位置
void place(void *bp, size_t asize) {
    // 拿到块的大小
    size_t csize = GET_SIZE(HDRP(bp));

    // bp块的大小相对放置的块的大小大于等于两个双字 （分出来的块放得下 header 和 footer）
    if ((csize - asize) >= (2*DSIZE)) {
        // 放置 header 和 footer，标记为 allocated
        // 分成两个块
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize-asize, 0));
        PUT(FTRP(bp), PACK(csize-asize, 0));
    }
    else {
        // 空间不够，就不分开了
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    // 创建初始的 empty heap
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0); // 对齐

    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); // 序言块头
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); // 序言块尾
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1)); // 结尾块

    heap_listp += 2 * WSIZE;

    // 如果使用 next fit 策略 则将上一个位置保存
    #ifdef NEXT_FIT
        rover = heap_listp;
    #endif
    // 拓展空闲空间
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;

    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char *bp;

    // 未初始化，进行初始化
    if (heap_listp == 0) {
        mm_init();
    }
    if (size == 0) return NULL;

    // 对请求空间的大小进行对齐
    // 如果请求的大小小于等于 DSIZE，请求两个DSIZE的大小
    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);

    // 可以找到合适的空闲块
    if ((bp = find_fit(asize)) != NULL) {
        // place() 函数用于将指定大小的内存块放置在空闲内存块的起始位置
        // 如果剩余空间足够大，就将其分割成一个新的空闲内存块
        // 将其放置到这个空闲块
        place(bp, asize);
        return bp;
    }

    // 找不到合适的空闲块 拓展堆空间
    // 系统调用昂贵，一次尽量多的申请内存空间
    extendsize = MAX(asize,CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    // 空指针
    if (bp == NULL)
        return;

    // 获取块大小
    size_t size = GET_SIZE(HDRP(bp));
    // 如果 heap_listp 没有初始化就初始化
    if (heap_listp == 0) {
        mm_init();
    }

    // 初始化为空闲块
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    // 尝试与附近的块合并
    coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    // 这一段是 lab 里直接有的，不用改 实现其他部分就可以了
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    // 另外malloc一块对应大小的内存 将原指针的内容 copy 进去 然后 free 掉原指针指向的块
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














