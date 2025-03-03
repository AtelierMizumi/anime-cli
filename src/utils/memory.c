#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"

void* safe_malloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

char* safe_strdup(const char *str) {
    if (!str) return NULL;
    
    char *dup = strdup(str);
    if (!dup) {
        fprintf(stderr, "Memory allocation failed in strdup\n");
        exit(EXIT_FAILURE);
    }
    return dup;
}