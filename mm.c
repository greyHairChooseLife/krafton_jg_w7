#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memlib.h"
#include "mm.h"

team_t team = {
    /* Team name */
    "no-team",
    /* First member's full name */
    "no-one",
    /* First member's email address */
    "",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};

#define WSIZE 4              // "Word" and header/footer size(bytes)
#define DSIZE 8              // "Double Word" size
#define CHUNKSIZE (1 << 12)  // 4KB, heap extending size

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define COMBINE(blockSize, allocBit) ((blockSize) | (allocBit))

// read & write combination of <size + allocation bit> by reading word size
#define GET_COMBI(ptr) (*(unsigned int*)(ptr))
#define PUT_COMBI(ptr, blockSize) (*(unsigned int*)(ptr) = (blockSize))

// discard last 3 bits or remain last 1 bit
#define GET_SIZE(ptr) (GET_COMBI(ptr) & ~0x7)
#define GET_ALLOC(ptr) (GET_COMBI(ptr) & 0x1)

// given block ptr, get address of its header/footer
#define HEADER_P(ptr) ((char*)(ptr) - WSIZE)
#define FOOTER_P(ptr) ((char*)(ptr) + GET_SIZE(HEADER_P(ptr)) - DSIZE)

// given block ptr, get address of <next & prev> block's, which is BlockPtr
#define NEXT_BP(ptr) ((char*)(ptr) + GET_SIZE(((char*)(bp) - WSIZE)))
#define PREV_BP(ptr) ((char*)(ptr) - GET_SIZE(((char*)(bp) - DSIZE)))

int mm_init(void) { return 0; }

void* mm_malloc(size_t size) { return NULL; }

void mm_free(void* ptr) {}

void* mm_realloc(void* ptr, size_t size) { return NULL; }
