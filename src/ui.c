#define _GNU_SOURCE // Add this before includes to get strcasestr
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "ui.h"
#include "api.h"


#define MAX_QUERY_LENGTH 256
#define ENTER_KEY 10
#define ESC_KEY 27

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

char* ui_get_search_query() {
    clear();
    attron(COLOR_PAIR(1));
    mvprintw(1, 1, "Search anime: ");
    attroff(COLOR_PAIR(1));
    refresh();
    
    char *query = malloc(MAX_QUERY_LENGTH);
    if (!query) {
        return NULL;
    }
    
    echo(); // Show user input
    getstr(query);
    noecho();
    
    return query;
}

void ui_show_loading() {
    clear();
    attron(COLOR_PAIR(2));
    mvprintw(1, 1, "Searching...");
    attroff(COLOR_PAIR(2));
    refresh();
}

AnimeInfo* ui_select_anime(SearchResult *results) {
    if (!results || results->total_results <= 0) {
        clear();
        attron(COLOR_PAIR(3));
        mvprintw(1, 1, "No results found.");
        attroff(COLOR_PAIR(3));
        getch();
        return NULL;
    }
    
    int choice = 0;
    int scroll_offset = 0;
    int max_display = LINES - 5; // Reserve some lines for header/footer
    int c;
    char filter[MAX_QUERY_LENGTH] = "";
    int filter_pos = 0;
    bool filtering = false;
    
    while (1) {
        clear();
        int line = 1;
        
        // Show title
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(line++, 1, "Select anime:");
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
                mvprintw(line++, 1, "> %s (%d episodes)", 
                         results->results[i].title,
                         results->results[i].episodes);
                attroff(A_REVERSE | COLOR_PAIR(2));
            } else {
                mvprintw(line++, 3, "%s (%d episodes)", 
                         results->results[i].title,
                         results->results[i].episodes);
            }
            displayed++;
        }
        
        // Display instructions
        line = LINES - 2;
        attron(COLOR_PAIR(1));
        mvprintw(line++, 1, "Use UP/DOWN arrows to navigate, ENTER to select");
        mvprintw(line, 1, "Type to filter results, ESC to clear filter, Ctrl+C to quit");
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
        
        if (filtering && c == KEY_BACKSPACE) {
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
                // Create AnimeInfo struct for selected anime
                AnimeInfo *selected = malloc(sizeof(AnimeInfo));
                if (!selected) return NULL;
                
                selected->id = strdup(results->results[choice].id);
                selected->title = strdup(results->results[choice].title);
                selected->total_episodes = results->results[choice].episodes;
                
                // Extra fields initialized to NULL or 0
                selected->url = NULL;
                selected->image = NULL;
                selected->description = NULL;
                selected->release_date = NULL;
                selected->status = NULL;
                selected->genres = NULL;
                selected->genres_count = 0;
                selected->sub_or_dub = NULL;
                selected->episodes = NULL;
                
                return selected;
            case 'q':
                return NULL;
        }
    }
    
    return NULL;
}

Episode* ui_select_episode(AnimeInfo *anime) {
    if (!anime || !anime->episodes || anime->total_episodes <= 0) {
        clear();
        attron(COLOR_PAIR(3));
        mvprintw(1, 1, "No episodes available for this anime.");
        attroff(COLOR_PAIR(3));
        getch();
        return NULL;
    }
    
    int choice = 0;
    int scroll_offset = 0;
    int max_display = LINES - 5;
    int c;
    
    while (1) {
        clear();
        int line = 1;
        
        // Show title
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(line++, 1, "Anime: %s", anime->title);
        mvprintw(line++, 1, "Select episode:");
        attroff(COLOR_PAIR(1) | A_BOLD);
        line++;
        
        // Display episodes
        int displayed = 0;
        for (int i = scroll_offset; i < anime->total_episodes && displayed < max_display; i++) {
            if (i == choice) {
                attron(A_REVERSE | COLOR_PAIR(2));
                mvprintw(line++, 1, "> Episode %d", anime->episodes[i].number);
                attroff(A_REVERSE | COLOR_PAIR(2));
            } else {
                mvprintw(line++, 3, "Episode %d", anime->episodes[i].number);
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
                if (choice < anime->total_episodes - 1) choice++;
                if (choice >= scroll_offset + max_display) scroll_offset = choice - max_display + 1;
                break;
            case KEY_NPAGE: // Page Down
                choice += max_display;
                if (choice >= anime->total_episodes) choice = anime->total_episodes - 1;
                scroll_offset = choice - (choice % max_display);
                break;
            case KEY_PPAGE: // Page Up
                choice -= max_display;
                if (choice < 0) choice = 0;
                scroll_offset = choice - (choice % max_display);
                break;
            case ENTER_KEY:
                return &anime->episodes[choice];
            case 'q':
                return NULL;
        }
    }
    
    return NULL;
}

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

void ui_play_episode(StreamInfo *stream) {
    endwin(); // Temporarily exit ncurses mode
    
    if (!stream || !stream->sources || stream->sources_count == 0) {
        printf("No streaming sources available.\n");
        printf("Press Enter to continue...");
        getchar();
        
        // Reinitialize ncurses before returning
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(1);
        start_color();
        init_pair(1, COLOR_CYAN, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_RED, COLOR_BLACK);
        return;
    }
    
    // Find best quality stream (usually the first one)
    char command[4096] = {0}; // Increased buffer size for many subtitles
    
    printf("Debug - Stream URL: %s\n", stream->sources[0].url);
    printf("Debug - Is M3U8: %d\n", stream->sources[0].is_m3u8);
    printf("Debug - Subtitle count: %d\n", stream->subtitles_count);
    
    // Basic mpv command with video URL and improved options
    if (stream->sources[0].is_m3u8) {
        // HLS stream needs special handling
        snprintf(command, sizeof(command), 
                "mpv --force-window=immediate --cache=yes --demuxer-max-bytes=150M \"%s\"", 
                stream->sources[0].url);
    } else {
        snprintf(command, sizeof(command), "mpv \"%s\"", stream->sources[0].url);
    }

    // Add headers if provided
    if (stream->referer) {
        char header_cmd[256];
        snprintf(header_cmd, sizeof(header_cmd), " --http-header-fields=\"Referer: %s\"", stream->referer);
        strcat(command, header_cmd);
    }
    
    if (stream->user_agent) {
        char ua_cmd[256];
        snprintf(ua_cmd, sizeof(ua_cmd), " --user-agent=\"%s\"", stream->user_agent);
        strcat(command, ua_cmd);
    }
    
    // Add subtitle support
    if (stream->subtitles && stream->subtitles_count > 0) {
        // Find English subtitle index (prefer Crunchyroll if available)
        int english_sub_index = -1;
        int english_cr_index = -1;
        
        for (int i = 0; i < stream->subtitles_count; i++) {
            if (stream->subtitles[i].url && stream->subtitles[i].lang) {
                printf("Debug - Subtitle %d: %s (%s)\n", i, stream->subtitles[i].lang, stream->subtitles[i].url);
                
                // Skip thumbnail VTT files
                if (strstr(stream->subtitles[i].url, "thumbnails.vtt") != NULL) {
                    continue;
                }
                
                // Check for English subtitles
                if (strstr(stream->subtitles[i].lang, "English") != NULL) {
                    // Prefer Crunchyroll subtitles
                    if (strstr(stream->subtitles[i].lang, "Crunchyroll") != NULL) {
                        english_cr_index = i;
                    } else if (english_sub_index == -1) {
                        english_sub_index = i;
                    }
                }
            }
        }
        
        // Use Crunchyroll English subtitle if found, otherwise use any English
        int selected_sub_index = (english_cr_index != -1) ? english_cr_index : english_sub_index;
        
        // Add all valid subtitle files
        int valid_sub_count = 0;
        for (int i = 0; i < stream->subtitles_count; i++) {
            if (stream->subtitles[i].url && stream->subtitles[i].lang) {
                // Skip thumbnail VTT files
                if (strstr(stream->subtitles[i].url, "thumbnails") != NULL) {
                    continue;
                }
                
                valid_sub_count++;
                char sub_cmd[512];
                snprintf(sub_cmd, sizeof(sub_cmd), " --sub-file=\"%s\"", stream->subtitles[i].url);
                strcat(command, sub_cmd);
            }
        }
        
        // Select English subtitle by default if found
        if (selected_sub_index != -1) {
            for (int i = 0, valid_idx = 0; i < selected_sub_index; i++) {
                if (stream->subtitles[i].url && stream->subtitles[i].lang &&
                    strstr(stream->subtitles[i].url, "thumbnails") == NULL) {
                    valid_idx++;
                }
            }
            
            char select_cmd[32];
            snprintf(select_cmd, sizeof(select_cmd), " --sid=1"); // Start with first subtitle
            strcat(command, select_cmd);
        }
    }
    
    // Print the command for debugging
    printf("Debug - Command: %s\n", command);
    
    // Execute the command2
    system(command);
    
    printf("\nReturning to episode selection menu. Press Enter to continue...\n");
    getchar(); // Wait for user input before returning
    
    // Fully reinitialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(1);
    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
}