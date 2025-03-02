#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

// Utility functions can be added here as needed
#include "utils.h"

void handle_error(const char *message) {
    fprintf(stderr, "Error: %s\n", message);
    exit(EXIT_FAILURE);
}

char* parse_json_string(const char *json, const char *key) {
    char *result = NULL;
    char *start = strstr(json, key);
    if (start) {
        start += strlen(key) + 3; // Move past key and quotes
        char *end = strchr(start, '"');
        if (end) {
            size_t length = end - start;
            result = malloc(length + 1);
            if (result) {
                strncpy(result, start, length);
                result[length] = '\0';
            }
        }
    }
    return result;
}

int parse_json_int(const char *json, const char *key) {
    int result = 0;
    char *start = strstr(json, key);
    if (start) {
        start += strlen(key) + 1; // Move past key and colon
        result = atoi(start);
    }
    return result;
}