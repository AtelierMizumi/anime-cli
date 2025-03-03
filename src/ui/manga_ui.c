#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "manga_ui.h"
#include "common/input.h"
#include "common/display.h"
#include "../config.h"
#include "../api/manga.h"
#include "../utils/memory.h"

#define MAX_QUERY_LENGTH 256
#define ENTER_KEY 10
#define ESC_KEY 27
#define BACKSPACE_KEY 127

char* manga_ui_get_search_query() {
    clear();
    attron(COLOR_PAIR(1));
    mvprintw(1, 1, "Search manga: ");
    attroff(COLOR_PAIR(1));
    refresh();
    
    return ui_get_text_input(MAX_QUERY_LENGTH);
}

void* manga_ui_select_manga(SearchResult *results) {
    if (!results || results->total_results <= 0) {
        ui_show_error("No results found.");
        return NULL;
    }
    
    int choice = 0;
    int scroll_offset = 0;
    int max_display = LINES - 5;
    int c;
    char filter[MAX_QUERY_LENGTH] = "";
    int filter_pos = 0;
    bool filtering = false;
    
    while (1) {
        clear();
        int line = 1;
        
        // Show title
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(line++, 1, "Select manga:");
        attroff(COLOR_PAIR(1) | A_BOLD);
        line++;
        
        // Display filter if active
        if (filtering) {
            attron(COLOR_PAIR(2));
            mvprintw(line++, 1, "Filter: %s", filter);
            attroff(COLOR_PAIR(2));
        }
        
        // Display results
        int displayed = 0;
        for (int i = scroll_offset; i < results->total_results && displayed < max_display; i++) {
            // Skip if filtering and doesn't match
            if (filtering && filter[0] != '\0' && 
                strcasestr(results->results[i].title, filter) == NULL) {
                continue;
            }
            
            if (i == choice) {
                attron(A_REVERSE | COLOR_PAIR(2));
                mvprintw(line++, 1, "> %s (%d chapters)", 
                         results->results[i].title,
                         results->results[i].episodes_or_chapters);
                attroff(A_REVERSE | COLOR_PAIR(2));
            } else {
                mvprintw(line++, 3, "%s (%d chapters)", 
                         results->results[i].title,
                         results->results[i].episodes_or_chapters);
            }
            displayed++;
        }
        
        // Display instructions
        line = LINES - 2;
        attron(COLOR_PAIR(1));
        mvprintw(line++, 1, "Use UP/DOWN arrows to navigate, ENTER to select");
        mvprintw(line, 1, "Type to filter results, ESC to clear filter, 'q' to go back");
        attroff(COLOR_PAIR(1));
        
        refresh();
        
        c = getch();
        
        // Handle filtering mode
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
            (c >= '0' && c <= '9') || c == ' ' || c == '-') {
            filtering = true;
            if (filter_pos < MAX_QUERY_LENGTH - 1) {
                filter[filter_pos++] = c;
                filter[filter_pos] = '\0';
            }
            choice = scroll_offset; // Reset selection to first visible item
            continue;
        }
        
        if (filtering && (c == KEY_BACKSPACE || c == BACKSPACE_KEY)) {
            if (filter_pos > 0) {
                filter[--filter_pos] = '\0';
                if (filter_pos == 0) {
                    filtering = false;
                }
            }
            continue;
        }
        
        if (filtering && c == ESC_KEY) {
            filtering = false;
            filter[0] = '\0';
            filter_pos = 0;
            continue;
        }
        
        switch (c) {
            case KEY_UP:
                if (choice > 0) choice--;
                if (choice < scroll_offset) scroll_offset = choice;
                break;
            case KEY_DOWN:
                if (choice < results->total_results - 1) choice++;
                if (choice >= scroll_offset + max_display) scroll_offset = choice - max_display + 1;
                break;
            case KEY_NPAGE: // Page Down
                choice += max_display;
                if (choice >= results->total_results) choice = results->total_results - 1;
                scroll_offset = choice - (choice % max_display);
                break;
            case KEY_PPAGE: // Page Up
                choice -= max_display;
                if (choice < 0) choice = 0;
                scroll_offset = choice - (choice % max_display);
                break;
            case ENTER_KEY:
                // Get provider API and request detailed manga info
                const ProviderAPI* api = get_provider_api(get_current_provider());
                return api->get_manga_info(results->results[choice].id);
            case 'q':
                return NULL;
        }
    }
    
    return NULL;
}

void* manga_ui_select_chapter(MangaInfo *manga) {
    if (!manga || !manga->chapters || manga->total_chapters <= 0) {
        ui_show_error("No chapters available for this manga.");
        return NULL;
    }
    
    int choice = 0;
    int scroll_offset = 0;
    int max_display = LINES - 7; // Leave space for header and info
    int c;
    
    while (1) {
        clear();
        int line = 1;
        
        // Show title and info
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(line++, 1, "Manga: %s", manga->title);
        attroff(A_BOLD);
        mvprintw(line++, 1, "Status: %s", manga->status ? manga->status : "Unknown");
        mvprintw(line++, 1, "Total Chapters: %d", manga->total_chapters);
        attroff(COLOR_PAIR(1));
        line++;
        
        // Show chapter selection title
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(line++, 1, "Select chapter:");
        attroff(COLOR_PAIR(2) | A_BOLD);
        
        // Display chapters
        int displayed = 0;
        for (int i = scroll_offset; i < manga->total_chapters && displayed < max_display; i++) {
            if (i == choice) {
                attron(A_REVERSE | COLOR_PAIR(2));
                if (manga->chapters[i].title && strlen(manga->chapters[i].title) > 0) {
                    mvprintw(line++, 1, "> Chapter %d: %s", 
                             manga->chapters[i].number, 
                             manga->chapters[i].title);
                } else {
                    mvprintw(line++, 1, "> Chapter %d", 
                             manga->chapters[i].number);
                }
                attroff(A_REVERSE | COLOR_PAIR(2));
            } else {
                if (manga->chapters[i].title && strlen(manga->chapters[i].title) > 0) {
                    mvprintw(line++, 3, "Chapter %d: %s", 
                             manga->chapters[i].number,
                             manga->chapters[i].title);
                } else {
                    mvprintw(line++, 3, "Chapter %d", 
                             manga->chapters[i].number);
                }
            }
            displayed++;
        }
        
        // Display instructions
        line = LINES - 2;
        attron(COLOR_PAIR(1));
        mvprintw(line++, 1, "Use UP/DOWN arrows to navigate, ENTER to select");
        mvprintw(line, 1, "Press 'q' to go back, Ctrl+C to quit");
        attroff(COLOR_PAIR(1));
        
        refresh();
        
        c = getch();
        
        switch (c) {
            case KEY_UP:
                if (choice > 0) choice--;
                if (choice < scroll_offset) scroll_offset = choice;
                break;
            case KEY_DOWN:
                if (choice < manga->total_chapters - 1) choice++;
                if (choice >= scroll_offset + max_display) scroll_offset = choice - max_display + 1;
                break;
            case KEY_NPAGE: // Page Down
                choice += max_display;
                if (choice >= manga->total_chapters) choice = manga->total_chapters - 1;
                scroll_offset = choice - (choice % max_display);
                break;
            case KEY_PPAGE: // Page Up
                choice -= max_display;
                if (choice < 0) choice = 0;
                scroll_offset = choice - (choice % max_display);
                break;
            case ENTER_KEY:
                const ProviderAPI* api = get_provider_api(get_current_provider());
                return api->get_chapter_pages(manga->chapters[choice].id);
            case 'q':
                return NULL;
        }
    }
    
    return NULL;
}

void manga_ui_view_chapter(ChapterPages *pages) {
    if (!pages || !pages->page_urls || pages->page_count <= 0) {
        ui_show_error("No pages available for this chapter.");
        return;
    }
    
    // Save current terminal state
    endwin();
    
    // Create a temporary file with all image URLs
    FILE *temp_file = tmpfile();
    if (!temp_file) {
        fprintf(stderr, "Failed to create temporary file\n");
        
        // Restore terminal state
        refresh();
        return;
    }
    
    // Write image URLs to temp file
    for (int i = 0; i < pages->page_count; i++) {
        fprintf(temp_file, "%s\n", pages->page_urls[i]);
    }
    fflush(temp_file);
    
    // Get file descriptor
    int fd = fileno(temp_file);
    
    // Build command for image viewer
    char command[1024];
    
    // Choose image viewer based on availability
    // Try feh first (good for manga)
    snprintf(command, sizeof(command), 
             "feh -. --scale-down --draw-filename --image-bg black -f /dev/fd/%d 2>/dev/null || "
             "sxiv -a -f -p -i /dev/fd/%d 2>/dev/null || "
             "imv /dev/fd/%d 2>/dev/null || "
             "xdg-open /dev/fd/%d",
             fd, fd, fd, fd);
    
    // Execute the command
    int result = system(command);
    
    if (result != 0) {
        fprintf(stderr, "Failed to open image viewer. Make sure feh, sxiv, imv, or a default image viewer is installed.\n");
        fprintf(stderr, "Press Enter to continue...\n");
        getchar();
    }
    
    // Clean up
    fclose(temp_file);
    
    // Restore terminal state
    refresh();
}

void manga_ui_main_loop() {
    while (1) {
        // Get search query
        char *query = manga_ui_get_search_query();
        if (!query || strlen(query) == 0) {
            free(query);
            return; // Return to main menu
        }
        
        // Show loading indicator
        ui_show_loading("Searching manga...");
        
        // Search for manga
        SearchResult *results = manga_search(query);
        free(query);
        
        if (!results || results->total_results == 0) {
            ui_show_error("No manga found matching your query.");
            if (results) manga_free_search_results(results);
            continue;
        }
        
        // Let user select a manga
        MangaInfo *selected_manga = manga_ui_select_manga(results);
        manga_free_search_results(results);
        
        if (!selected_manga) {
            continue; // Return to search
        }
        
        // Let user select a chapter
        ChapterPages *chapter_pages = manga_ui_select_chapter(selected_manga);
        manga_free_info(selected_manga);
        
        if (!chapter_pages) {
            continue; // Return to search
        }
        
        // View the chapter
        manga_ui_view_chapter(chapter_pages);
        manga_free_chapter_pages(chapter_pages);
    }
}