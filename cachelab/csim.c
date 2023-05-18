#include "cachelab.h"
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// 命令行参数
int h, v, s, E, b, S;
int hit_count, miss_count, eviction_count;
typedef struct 
{
    int valid_bits;
    int tag;
    int lru_counter;
} cache_line, *cahce_set, **cache;

char t[1000];

cache _cache_ = NULL;

void printUsage()
{
    printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n"
            "Options:\n"
            "  -h         Print this help message.\n"
            "  -v         Optional verbose flag.\n"
            "  -s <num>   Number of set index bits.\n"
            "  -E <num>   Number of lines per set.\n"
            "  -b <num>   Number of block offset bits.\n"
            "  -t <file>  Trace file.\n\n"
            "Examples:\n"
            "  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n"
            "  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

void init_cache() 
{
    _cache_ = (cache) malloc(sizeof(cahce_set) * S);
    for (int i = 0; i < S; ++i) {
        _cache_[i] = (cahce_set) malloc(sizeof(cahce_set) * E);
        for (int j = 0; j < E; ++j) {
            _cache_[i][j].valid_bits = 0;
            _cache_[i][j].tag = -1;
            _cache_[i][j].lru_counter = -1;
        }
    }
}

void lru(uint64_t address) {
    uint32_t tag = (address >> b) >> s;
    uint32_t index = (address >> b) & ((1 << s) - 1);
    // check if hit
    for (int i = 0; i < E; i++) {
        cache_line *cl = &(_cache_[index][i]);
        if (cl->valid_bits == 1 && cl->tag == tag) {
            // hit
            hit_count++;
            cl->lru_counter = 0;
            if (v) {
                printf("hit ");
            }
            return;
        }
    }
    // check if miss
    for (int i = 0; i < E; i++) {
        cache_line *cl = &(_cache_[index][i]);
        if (cl->valid_bits == 0) {
            // miss
            miss_count++;
            cl->valid_bits = 1;
            cl->tag = tag;
            cl->lru_counter = 0;
            if (v) {
                printf("miss ");
            }
            return;
        }
    }
    // not hit and miss, evict
    eviction_count++;
    miss_count++;
    int maxCounter = -2;
    int evictIndex = 0;
    for (int i = 0; i < E; i++) {
        cache_line *cl = &(_cache_[index][i]);
        if (cl->lru_counter > maxCounter) {
            maxCounter = cl->lru_counter;
            evictIndex = i;
        }
    }
    cache_line *evict_line = &(_cache_[index][evictIndex]);
    evict_line->tag = tag;
    evict_line->lru_counter = 0;
    printf("miss eviction ");
}

void parse_trace(FILE* fp) {
    char op;
    uint64_t address;
    int size;
    while (fscanf(fp, " %c %lx,%d\n", &op, &address, &size) > 0) {
        printf("%c %lx %d ", op, address, size);
        switch (op)
        {
        // data load
        case 'L':
            lru(address);
            printf("\n");
            break;
        // data modify 
        case 'M':
            lru(address);
            // modify完事还要store回去
        // data store
        case 'S': 
            lru(address);
            printf("\n");
            break;
        }
    }
}

int main(int argc, char* argv[])
{
    h = 0;
    v = 0;
    hit_count = miss_count = eviction_count = 0;
    int opt;
    // getopt 第三个参数中，不可省略的选项字符后要跟冒号，这里h和v可省略
	while(-1 != (opt = (getopt(argc, argv, "hvs:E:b:t:"))))
	{
		switch(opt)
		{
			case 'h':
				h = 1;
                printUsage();
				break;
			case 'v':
				v = 1;
				break;
			case 's':
				s = atoi(optarg);
				break;
			case 'E':
				E = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
				break;
			case 't':
				strcpy(t, optarg);
				break;
			default:
				printUsage();
				break;
		}
	}

    if (s <= 0 || E <= 0 || b <= 0 || t == NULL) // 如果选项参数不合格就退出
	     return -1;
	S = 1 << s;                // S=2^s

	FILE* fp = fopen(t, "r");
	if (fp == NULL) {
		printf("open error");
		exit(-1);
	}

    init_cache();
    parse_trace(fp);

    // 关文件
    fclose(fp);
    free(_cache_);
    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}
