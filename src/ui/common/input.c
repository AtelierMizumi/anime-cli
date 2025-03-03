#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <ctype.h>
#include "input.h"

char* ui_get_text_input(int max_length) {
    char *input = malloc(max_length);
    if (!input) {
        return NULL;
    }
    
    echo(); // Show user input
    getstr(input);
    noecho();
    
    return input;
}

bool ui_get_confirmation(const char *prompt) {
    clear();
    mvprintw(1, 1, "%s (y/n): ", prompt);
    refresh();
    
    int c;
    do {
        c = getch();
        c = tolower(c);
    } while (c != 'y' && c != 'n');
    
    return (c == 'y');
}