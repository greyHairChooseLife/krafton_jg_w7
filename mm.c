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

// mm_init - initialize the malloc package.
int mm_init(void) { return 0; }

// mm_malloc - Allocate a block by incrementing the brk pointer.
//     Always allocate a block whose size is a multiple of the alignment.
void* mm_malloc(size_t size) { return NULL; }

// mm_free - Freeing a block does nothing.
void mm_free(void* ptr) {}

// mm_realloc - Implemented simply in terms of mm_malloc and mm_free
void* mm_realloc(void* ptr, size_t size) { return NULL; }
