#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "ui.h"
#include "common/display.h"
#include "common/input.h"
#include "../config.h"   // Add this line to include config.h

void ui_init() {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(1); // Show cursor
    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
}

void ui_cleanup() {
    endwin();
}

ContentSelectionOption ui_content_selection() {
    int choice = 0;
    int c;
    
    while (1) {
        clear();
        int line = 1;
        
        // Show title
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(line++, 1, "Anime CLI");
        mvprintw(line++, 1, "Select content type:");
        attroff(COLOR_PAIR(1) | A_BOLD);
        line++;
        
        // Display options
        if (choice == 0) {
            attron(A_REVERSE | COLOR_PAIR(2));
            mvprintw(line++, 1, "> Anime");
            attroff(A_REVERSE | COLOR_PAIR(2));
        } else {
            mvprintw(line++, 3, "Anime");
        }
        
        if (choice == 1) {
            attron(A_REVERSE | COLOR_PAIR(2));
            mvprintw(line++, 1, "> Manga");
            attroff(A_REVERSE | COLOR_PAIR(2));
        } else {
            mvprintw(line++, 3, "Manga");
        }
        
        if (choice == 2) {
            attron(A_REVERSE | COLOR_PAIR(2));
            mvprintw(line++, 1, "> Exit");
            attroff(A_REVERSE | COLOR_PAIR(2));
        } else {
            mvprintw(line++, 3, "Exit");
        }
        
        // Display instructions
        line = LINES - 2;
        attron(COLOR_PAIR(1));
        mvprintw(line++, 1, "Use UP/DOWN arrows to navigate, ENTER to select");
        attroff(COLOR_PAIR(1));
        
        refresh();
        
        c = getch();
        
        switch (c) {
            case KEY_UP:
                if (choice > 0) choice--;
                break;
            case KEY_DOWN:
                if (choice < 2) choice++;
                break;
            case 10: // Enter key
                switch (choice) {
                    case 0:
                        return CONTENT_SELECTION_ANIME;
                    case 1:
                        return CONTENT_SELECTION_MANGA;
                    case 2:
                        return CONTENT_SELECTION_EXIT;
                }
                break;
        }
    }
    
    return CONTENT_SELECTION_EXIT;
}

// ... existing code ...

ProviderSelectionResult ui_provider_selection(ContentType content_type) {
    ProviderSelectionResult result;
    result.selected_provider = get_current_provider();
    result.canceled = false;
    
    int count;
    const char **providers = get_available_providers(content_type, &count);
    
    if (count == 0) {
        ui_show_error("No providers available for this content type");
        result.canceled = true;
        return result;
    }
    
    // If there's only one provider, just use that one
    if (count == 1) {
        for (int i = 0; i < PROVIDER_COUNT; i++) {
            if (strcmp(provider_type_to_string(i), providers[0]) == 0) {
                result.selected_provider = i;
                break;
            }
        }
        return result;
    }
    
    int choice = 0;
    int c;
    
    while (1) {
        clear();
        int line = 1;
        
        // Show title
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(line++, 1, "Select Provider for %s:", content_type_to_string(content_type));
        attroff(COLOR_PAIR(1) | A_BOLD);
        line++;
        
        // Display provider options
        for (int i = 0; i < count; i++) {
            if (i == choice) {
                attron(A_REVERSE | COLOR_PAIR(2));
                mvprintw(line++, 1, "> %s", providers[i]);
                attroff(A_REVERSE | COLOR_PAIR(2));
            } else {
                mvprintw(line++, 3, "%s", providers[i]);
            }
        }
        
        // Back option
        if (choice == count) {
            attron(A_REVERSE | COLOR_PAIR(2));
            mvprintw(line++, 1, "> Back");
            attroff(A_REVERSE | COLOR_PAIR(2));
        } else {
            mvprintw(line++, 3, "Back");
        }
        
        // Display instructions
        line = LINES - 2;
        attron(COLOR_PAIR(1));
        mvprintw(line++, 1, "Use UP/DOWN arrows to navigate, ENTER to select");
        attroff(COLOR_PAIR(1));
        
        refresh();
        
        c = getch();
        
        switch (c) {
            case KEY_UP:
                if (choice > 0) choice--;
                break;
            case KEY_DOWN:
                if (choice < count) choice++;
                break;
            case 10: // Enter key
                if (choice == count) {
                    result.canceled = true;
                    return result;
                }
                
                // Find the provider type that matches the selected name
                for (int i = 0; i < PROVIDER_COUNT; i++) {
                    if (strcmp(provider_type_to_string(i), providers[choice]) == 0) {
                        result.selected_provider = i;
                        return result;
                    }
                }
                
                // If we get here, something went wrong
                result.canceled = true;
                return result;
        }
    }
}