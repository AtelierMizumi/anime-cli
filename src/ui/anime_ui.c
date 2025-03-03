#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "anime_ui.h"
#include "common/input.h"
#include "common/display.h"
#include "../api/providers/zoro.h"
#include "../api/anime.h"
#include "../config.h"

#define MAX_QUERY_LENGTH 256
#define ENTER_KEY 10
#define ESC_KEY 27

char* anime_ui_get_search_query() {
    clear();
    attron(COLOR_PAIR(1));
    mvprintw(1, 1, "Search anime: ");
    attroff(COLOR_PAIR(1));
    refresh();
    
    return ui_get_text_input(MAX_QUERY_LENGTH);
}

void* anime_ui_select_anime(SearchResult *results) {
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
                         results->results[i].episodes_or_chapters);
                attroff(A_REVERSE | COLOR_PAIR(2));
            } else {
                mvprintw(line++, 3, "%s (%d episodes)", 
                         results->results[i].title,
                         results->results[i].episodes_or_chapters);
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
                // Get provider API and request detailed anime info
                const ProviderAPI* api = get_provider_api(get_current_provider());
                return api->get_anime_info(results->results[choice].id);
            case 'q':
                return NULL;
        }
    }
    
    return NULL;
}

// ... existing code ...

void* anime_ui_select_episode(AnimeInfo *anime) {
    if (!anime || !anime->episodes || anime->total_episodes <= 0) {
        ui_show_error("No episodes available for this anime.");
        return NULL;
    }
    
    int choice = 0;
    int scroll_offset = 0;
    int max_display = LINES - 7; // Reserve space for header and info
    int c;
    
    while (1) {
        clear();
        int line = 1;
        
        // Show title and info
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(line++, 1, "Anime: %s", anime->title);
        if (anime->status)
            mvprintw(line++, 1, "Status: %s", anime->status);
        attroff(A_BOLD);
        mvprintw(line++, 1, "Episodes: %d", anime->total_episodes);
        attroff(COLOR_PAIR(1));
        line++;
        
        // Show episode selection title
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(line++, 1, "Select episode:");
        attroff(COLOR_PAIR(2) | A_BOLD);
        
        // Display episodes
        int displayed = 0;
        for (int i = scroll_offset; i < anime->total_episodes && displayed < max_display; i++) {
            if (i == choice) {
                attron(A_REVERSE | COLOR_PAIR(2));
                if (anime->episodes[i].title)
                    mvprintw(line++, 1, "> Episode %d: %s", 
                             anime->episodes[i].number,
                             anime->episodes[i].title);
                else
                    mvprintw(line++, 1, "> Episode %d", 
                             anime->episodes[i].number);
                attroff(A_REVERSE | COLOR_PAIR(2));
            } else {
                if (anime->episodes[i].title)
                    mvprintw(line++, 3, "Episode %d: %s", 
                             anime->episodes[i].number,
                             anime->episodes[i].title);
                else
                    mvprintw(line++, 3, "Episode %d", 
                             anime->episodes[i].number);
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
                // Get the episode ID and fetch stream info
                return anime->episodes[choice].id;
            case 'q':
                return NULL;
        }
    }
    
    return NULL;
}

void anime_ui_play_episode(StreamInfo *stream) {
    if (!stream || !stream->sources || stream->sources_count == 0) {
        ui_show_error("No streaming sources available.");
        return;
    }
    
    // Save current terminal state and exit ncurses mode
    endwin();
    
    printf("Loading episode...\n");
    
    // Build mpv command with URL and headers
    char command[4096] = {0};
    
    // Basic mpv command with video URL and improved options
    snprintf(command, sizeof(command), 
             "mpv --force-window=immediate --cache=yes --demuxer-max-bytes=150M \"%s\"", 
             stream->sources[0].url);

    // Add headers if provided
    if (stream->referer) {
        char header_cmd[512];
        snprintf(header_cmd, sizeof(header_cmd), 
                 " --http-header-fields=\"Referer: %s\"", stream->referer);
        strcat(command, header_cmd);
    }

    if (stream->user_agent) {
        char ua_cmd[512];
        snprintf(ua_cmd, sizeof(ua_cmd), 
                 " --user-agent=\"%s\"", stream->user_agent);
        strcat(command, ua_cmd);
    }

    // Add subtitle support with improved handling
    if (stream->subtitles && stream->subtitles_count > 0) {
        // Find English subtitle for default selection
        int english_sub_index = -1;
        
        for (int i = 0; i < stream->subtitles_count; i++) {
            if (stream->subtitles[i].url && stream->subtitles[i].lang) {
                // Skip thumbnail VTT files
                if (strstr(stream->subtitles[i].url, "thumbnails") != NULL) {
                    continue;
                }
                
                // Look for English subtitles
                if (strstr(stream->subtitles[i].lang, "English") != NULL) {
                    english_sub_index = i;
                }
            }
        }
        
        // Add all valid subtitle files
        int valid_sub_count = 0;
        for (int i = 0; i < stream->subtitles_count; i++) {
            if (stream->subtitles[i].url && stream->subtitles[i].lang) {
                // Skip thumbnail VTT files
                if (strstr(stream->subtitles[i].url, "thumbnails") != NULL) {
                    continue;
                }
                
                valid_sub_count++;
                char sub_cmd[1024];
                // Only use --sub-file without --sub-name for better compatibility
                snprintf(sub_cmd, sizeof(sub_cmd), 
                        " --sub-file=\"%s\"", 
                        stream->subtitles[i].url);
                strcat(command, sub_cmd);
                
                // Print language info to console for reference
                printf("Subtitle %d: %s\n", valid_sub_count, stream->subtitles[i].lang);
            }
        }
        
        // Select English subtitle by default if found
        if (english_sub_index >= 0) {
            char sid_cmd[32];
            // Convert from index to 1-based sid number, accounting for valid subs
            int sid = 1;
            for (int i = 0; i < english_sub_index; i++) {
                if (stream->subtitles[i].url && 
                    stream->subtitles[i].lang && 
                    strstr(stream->subtitles[i].url, "thumbnails") == NULL) {
                    sid++;
                }
            }
            snprintf(sid_cmd, sizeof(sid_cmd), " --sid=%d", sid);
            strcat(command, sid_cmd);
            printf("Default subtitle: English (sid=%d)\n", sid);
        }
        
        // Enable subtitle visibility by default
        strcat(command, " --sub-visibility=yes");
    }
    
    // Execute the command
    system(command);
    
    printf("Returning to episode selection. Press Enter to continue...\n");
    getchar();
    
    // Restore terminal state
    refresh();
}

void anime_ui_main_loop() {
    while (1) {
        // Get search query
        char *query = anime_ui_get_search_query();
        if (!query || strlen(query) == 0) {
            free(query);
            return; // Return to main menu
        }
        
        // Show loading indicator
        ui_show_loading("Searching anime...");
        
        // Search for anime
        SearchResult *results = anime_search(query);
        free(query);
        
        if (!results || results->total_results == 0) {
            ui_show_error("No anime found matching your query.");
            if (results) anime_free_search_results(results);
            continue;
        }
        
        // Let user select an anime
        AnimeInfo *selected_anime = anime_ui_select_anime(results);
        anime_free_search_results(results);
        
        if (!selected_anime) {
            continue; // Return to search
        }
        
        // Loop for selecting and watching episodes
        int stay_in_episode_menu = 1;
        while (stay_in_episode_menu) {
            // Let user select an episode
            char *episode_id = anime_ui_select_episode(selected_anime);
            
            if (episode_id) {
                // Get streaming link for the episode
                ui_show_loading("Getting stream data...");
                StreamInfo *stream_info = anime_get_episode_stream(episode_id, "vidstreaming");
                
                if (stream_info && stream_info->sources_count > 0) {
                    // Play the episode
                    anime_ui_play_episode(stream_info);
                    anime_free_stream_info(stream_info);
                    // After playing, we'll loop back to episode selection
                } else {
                    ui_show_error("Failed to get streaming link.");
                    // Still stay in the episode menu
                }
            } else {
                // User cancelled episode selection, exit the loop
                stay_in_episode_menu = 0;
            }
        }
        
        // Clean up anime info before returning to search
        anime_free_info(selected_anime);
    }
}