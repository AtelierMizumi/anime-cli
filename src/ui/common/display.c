#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include "display.h"
#include "../../config.h"

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

void display_search_result_item(int y, int x, const SearchResultItem *item, bool selected) {
    if (selected) {
        attron(A_REVERSE);
    }

    // Clear the line
    mvhline(y, x, ' ', COLS - x);
    
    // Display title with limited width
    int max_title_width = COLS - x - 20; // Reserve space for episode/chapter count
    mvprintw(y, x, "%.*s", max_title_width, item->title ? item->title : "Unknown Title");
    
    // Format and display episode/chapter count based on content type
    if (item->content_type == CONTENT_ANIME) {
        if (item->episodes_or_chapters > 0) {
            mvprintw(y, COLS - 15, "[%d Episodes]", item->episodes_or_chapters);
        } else {
            mvprintw(y, COLS - 15, "[Unknown]");
        }
    } else if (item->content_type == CONTENT_MANGA) {
        // For manga, we want to be more descriptive
        if (item->episodes_or_chapters > 1) {
            mvprintw(y, COLS - 15, "[%d Chapters]", item->episodes_or_chapters);
        } else if (item->episodes_or_chapters == 1) {
            // This could be a manga with just volume info or unknown chapter count
            mvprintw(y, COLS - 15, "[Chapters]");
        } else {
            mvprintw(y, COLS - 15, "[Unknown]");
        }
    }
    
    if (selected) {
        attroff(A_REVERSE);
    }
}