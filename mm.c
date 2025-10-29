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
    "no-one",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};

#define WSIZE 4              // "Word" and header/footer size(bytes)
#define DSIZE 8              // "Double Word" size
#define CHUNKSIZE (1 << 12)  // 4KB, heap extending size

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) > (y) ? (y) : (x))
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
#define NEXT_BP(ptr) ((char*)(ptr) + GET_SIZE(((char*)(ptr) - WSIZE)))
#define PREV_BP(ptr) ((char*)(ptr) - GET_SIZE(((char*)(ptr) - DSIZE)))

static char* heap_listp;  // heap area starting point

typedef struct {
    char* kinds;
    size_t sizes;
} blockStat;

static void checker(char* isExtended, size_t size) {
    return;
    void* ptrCurr = heap_listp + DSIZE;

    int blockCount = 0;
    int freeCount = 0;
    int freeAmount = 0;
    int allocCount = 0;
    int allocAmount = 0;
    blockStat blocks[10];
    int idx = 0;

    // adjusted block size
    size_t asize;
    if (strcmp(isExtended, "Free") == 0)
        asize = size;
    else if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((size + DSIZE + (DSIZE - 1)) / DSIZE);

    do {
        blockCount++;
        int curAlloc = GET_ALLOC(HEADER_P(ptrCurr));
        int curSize = GET_SIZE(HEADER_P(ptrCurr));

        if (curAlloc == 1) {
            allocCount++;
            allocAmount += curSize;
            blocks[idx].kinds = "allocated";
            blocks[idx].sizes = curSize;
        } else {
            freeCount++;
            freeAmount += curSize;
            blocks[idx].kinds = "free";
            blocks[idx].sizes = curSize;
        }
        idx++;

        ptrCurr = NEXT_BP(ptrCurr);
    } while (!(GET_SIZE(HEADER_P(ptrCurr)) == 0 &&
               GET_ALLOC(HEADER_P(ptrCurr)) == 1));
    blocks[idx].kinds = "EPILOGUE";
    blocks[idx].sizes = GET_SIZE(HEADER_P(ptrCurr));
    while (++idx < 10) {
        blocks[idx].kinds = "";
        blocks[idx].sizes = 0;
    }

    printf("(%s)REPORT: ------------------------ orig size (%zu) -> a (%zu)\n",
           isExtended, size, asize);
    for (int i = 0; i <= blockCount; i++)
        printf("    %d: Kind = %s, Size = %zu\n", i, blocks[i].kinds,
               blocks[i].sizes);
    printf("\n");
}

static void* coalesce(void* bp) {
    size_t prev_alloc = GET_ALLOC(FOOTER_P(PREV_BP(bp)));
    size_t next_alloc = GET_ALLOC(HEADER_P(NEXT_BP(bp)));
    size_t size = GET_SIZE(HEADER_P(bp));

    if (prev_alloc && next_alloc) {
        return bp;
    } else if (prev_alloc && !next_alloc) {
        size += GET_SIZE(HEADER_P(NEXT_BP(bp)));
        PUT_COMBI(HEADER_P(bp), COMBINE(size, 0));
        PUT_COMBI(FOOTER_P(bp), COMBINE(size, 0));
    } else if (!prev_alloc && next_alloc) {
        size += GET_SIZE(HEADER_P(PREV_BP(bp)));
        PUT_COMBI(FOOTER_P(bp), COMBINE(size, 0));  // no need to move
        PUT_COMBI(HEADER_P(PREV_BP(bp)),
                  COMBINE(size, 0));  // to the prev block
        bp = PREV_BP(bp);

    } else {
        size +=
            GET_SIZE(HEADER_P(PREV_BP(bp))) + GET_SIZE(FOOTER_P(NEXT_BP(bp)));
        PUT_COMBI(HEADER_P(PREV_BP(bp)), COMBINE(size, 0));
        PUT_COMBI(FOOTER_P(NEXT_BP(bp)), COMBINE(size, 0));
        bp = PREV_BP(bp);
    }

    return bp;
}

// extend heap size
// Returns NULL if failed, or Block Pointer that might be coalesced.
static void* extend_heap(size_t words) {
    char* bp;
    size_t size;

    // align
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;

    // set break and get block pointer
    if ((long)(bp = mem_sbrk(size)) == -1) return NULL;

    PUT_COMBI(HEADER_P(bp), COMBINE(size, 0));
    PUT_COMBI(FOOTER_P(bp), COMBINE(size, 0));
    PUT_COMBI(HEADER_P(NEXT_BP(bp)), COMBINE(0, 1));  // epilogue
    return coalesce(bp);
}

int mm_init(void) {
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void*)-1) return -1;

    PUT_COMBI(heap_listp, 0);
    PUT_COMBI(heap_listp + (1 * WSIZE), COMBINE(DSIZE, 1));
    PUT_COMBI(heap_listp + (2 * WSIZE), COMBINE(DSIZE, 1));
    PUT_COMBI(heap_listp + (3 * WSIZE), COMBINE(0, 1));
    heap_listp += DSIZE;

    if (extend_heap(CHUNKSIZE / WSIZE) == NULL) return -1;

    return 0;
}

// First-fit search
static void* find_fit(size_t asize) {
    void* bp;

    for (bp = heap_listp; GET_SIZE(HEADER_P(bp)) > 0; bp = NEXT_BP(bp))
        if ((!GET_ALLOC(HEADER_P(bp))) && (asize <= GET_SIZE(HEADER_P(bp))))
            return bp;

    return NULL;
}

static void place(void* bp, size_t asize) {
    size_t csize = GET_SIZE(HEADER_P(bp));

    // at least one minimal block, split it
    if ((csize - asize) >= (2 * DSIZE)) {
        PUT_COMBI(HEADER_P(bp), COMBINE(asize, 1));
        PUT_COMBI(FOOTER_P(bp), COMBINE(asize, 1));
        bp = NEXT_BP(bp);
        PUT_COMBI(HEADER_P(bp), COMBINE(csize - asize, 0));
        PUT_COMBI(FOOTER_P(bp), COMBINE(csize - asize, 0));
    } else {
        PUT_COMBI(HEADER_P(bp), COMBINE(csize, 1));
        PUT_COMBI(FOOTER_P(bp), COMBINE(csize, 1));
    }
}

void* mm_malloc(size_t size) {
    size_t asize;       // adjusted block size
    size_t extendSize;  // if no fit, extend this size
    char* bp;

    if (!size) return NULL;

    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((size + DSIZE + (DSIZE - 1)) / DSIZE);

    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        checker("Found", size);
        return bp;
    }

    // no fitted block found.
    extendSize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendSize / WSIZE)) == NULL) return NULL;

    place(bp, asize);
    checker("Extend", size);
    return bp;
}

void mm_free(void* ptr) {
    size_t size = GET_SIZE(HEADER_P(ptr));

    PUT_COMBI(HEADER_P(ptr), COMBINE(size, 0));
    PUT_COMBI(FOOTER_P(ptr), COMBINE(size, 0));

    coalesce(ptr);
    checker("Free", size);
}

void* mm_realloc(void* ptr, size_t size) {
    if (!size) {
        mm_free(ptr);
        return NULL;
    }

    size_t currSize = GET_SIZE(HEADER_P(ptr));
    size_t asize = DSIZE * ((size + DSIZE + (DSIZE - 1)) / DSIZE);

    if (asize == currSize) {
        checker("Realloc - same case", size);
        return ptr;
    }

    if (asize < currSize) {
        if (currSize - asize >= 2 * DSIZE) {
            PUT_COMBI(HEADER_P(ptr), COMBINE(asize, 1));
            PUT_COMBI(FOOTER_P(ptr), COMBINE(asize, 1));
            PUT_COMBI(HEADER_P(NEXT_BP(ptr)), COMBINE(currSize - asize, 0));
            PUT_COMBI(FOOTER_P(NEXT_BP(ptr)), COMBINE(currSize - asize, 0));
        }
        checker("Realloc - less case", size);
        return ptr;
    }

    /* // check if adjacent blocks are free */
    /* // find smallest */
    /* // not enough for asize => mm_malloc */
    void* newPtr;
    size_t prevAlloc = GET_ALLOC(FOOTER_P(PREV_BP(ptr)));
    size_t prevSize = GET_SIZE(FOOTER_P(PREV_BP(ptr)));
    size_t nextAlloc = GET_ALLOC(HEADER_P(NEXT_BP(ptr)));
    size_t nextSize = GET_SIZE(HEADER_P(NEXT_BP(ptr)));

    size_t withPrevSize = currSize + prevSize;
    size_t withNextSize = currSize + nextSize;
    size_t withBothSize = currSize + prevSize + nextSize;

    char unionBlock = 0;

    /* if (!prevAlloc && withPrevSize >= asize) unionBlock = 'p'; */
    if ((!nextAlloc && withNextSize >= asize && withNextSize < withPrevSize) ||
        (!nextAlloc && withNextSize >= asize && !unionBlock))
        unionBlock = 'n';
    /* if (!prevAlloc && !nextAlloc && withBothSize >= asize) unionBlock = 'b';
     */

    /* if (unionBlock == 'p' || unionBlock == 'b') { */
    /*     newPtr = PREV_BP(ptr); */
    /*     place(newPtr, asize); */
    /*     memcpy(newPtr, ptr, currSize - DSIZE); */
    /*     return newPtr; */
    /* } */
    /* if (unionBlock == 'n') { */
    /*     PUT_COMBI(HEADER_P(ptr), COMBINE(withNextSize, 0)); */
    /*     place(ptr, asize); */
    /*     return ptr; */
    /* } */

    newPtr = mm_malloc(size);
    memcpy(newPtr, ptr, currSize - DSIZE);
    mm_free(ptr);
    return newPtr;
}
