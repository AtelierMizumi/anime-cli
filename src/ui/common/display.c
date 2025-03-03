#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include "display.h"

void ui_show_error(const char *message) {
    clear();
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(1, 1, "Error:");
    attroff(A_BOLD);
    mvprintw(2, 1, "%s", message);
    mvprintw(4, 1, "Press any key to continue...");
    attroff(COLOR_PAIR(3));
    refresh();
    getch();
}

void ui_show_loading(const char *message) {
    clear();
    attron(COLOR_PAIR(2));
    mvprintw(1, 1, "%s", message ? message : "Loading...");
    attroff(COLOR_PAIR(2));
    refresh();
}

void ui_draw_progress_bar(int percentage, int width) {
    int fill_width = (percentage * width) / 100;
    
    attron(COLOR_PAIR(2));
    mvprintw(getcury(stdscr), getcurx(stdscr), "[");
    for (int i = 0; i < width; i++) {
        if (i < fill_width) {
            addch('=');
        } else {
            addch(' ');
        }
    }
    mvprintw(getcury(stdscr), getcurx(stdscr), "] %d%%", percentage);
    attroff(COLOR_PAIR(2));
}