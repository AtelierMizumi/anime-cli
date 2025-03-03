#ifndef ANIME_UI_H
#define ANIME_UI_H

#include "../api/anime.h"

// Get search query from user
char* anime_ui_get_search_query();

// Display anime search results and let user select one
void* anime_ui_select_anime(SearchResult *results);

// Display anime episodes and let user select one
void* anime_ui_select_episode(AnimeInfo *anime);

// Play episode with streaming information
void anime_ui_play_episode(StreamInfo *stream);

// Main anime UI loop
void anime_ui_main_loop();

#endif /* ANIME_UI_H */