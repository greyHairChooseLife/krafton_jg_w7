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

int mm_init(void) { return 0; }

void* mm_malloc(size_t size) { return NULL; }

void mm_free(void* ptr) {}

void* mm_realloc(void* ptr, size_t size) { return NULL; }
