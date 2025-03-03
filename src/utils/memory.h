#ifndef MEMORY_H
#define MEMORY_H

#include <stdlib.h>

// Safe allocation that checks for NULL
void* safe_malloc(size_t size);

// Safe string duplication
char* safe_strdup(const char *str);

#endif /* MEMORY_H */