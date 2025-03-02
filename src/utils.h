#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void handle_error(const char *message);
char* parse_json_string(const char *json, const char *key);
int parse_json_int(const char *json, const char *key);

#endif /* UTILS_H */