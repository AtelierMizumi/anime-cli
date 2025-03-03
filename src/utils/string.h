#ifndef STRING_H
#define STRING_H

#include <stdbool.h>

// Case-insensitive string search
char* case_insensitive_strstr(const char *haystack, const char *needle);

// URL encode a string
char* url_encode(const char *str);

#endif /* STRING_H */