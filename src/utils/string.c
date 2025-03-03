#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "string.h"
#include "memory.h"

char* case_insensitive_strstr(const char *haystack, const char *needle) {
    if (!haystack || !needle) return NULL;
    
    size_t needle_len = strlen(needle);
    size_t haystack_len = strlen(haystack);
    
    if (needle_len > haystack_len) return NULL;
    
    for (size_t i = 0; i <= haystack_len - needle_len; i++) {
        if (strncasecmp(haystack + i, needle, needle_len) == 0) {
            return (char*)(haystack + i);
        }
    }
    
    return NULL;
}

char* url_encode(const char *str) {
    if (!str) return NULL;
    
    const char *hex = "0123456789ABCDEF";
    size_t len = strlen(str);
    
    // Worst case: every character needs to be encoded (3x original size)
    char *encoded = safe_malloc(3 * len + 1);
    if (!encoded) return NULL;
    
    size_t pos = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = str[i];
        
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            // Safe characters can be included as-is
            encoded[pos++] = c;
        } else if (c == ' ') {
            // Space becomes a plus sign or %20
            encoded[pos++] = '+';
        } else {
            // Unsafe characters are percent-encoded
            encoded[pos++] = '%';
            encoded[pos++] = hex[(c >> 4) & 0xF];
            encoded[pos++] = hex[c & 0xF];
        }
    }
    
    encoded[pos] = '\0';
    return encoded;
}