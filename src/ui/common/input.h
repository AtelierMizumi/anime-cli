#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

// Get text input from user
char* ui_get_text_input(int max_length);

// Get yes/no confirmation from user
bool ui_get_confirmation(const char *prompt);

#endif /* INPUT_H */