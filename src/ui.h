#ifndef UI_H
#define UI_H

#include "api.h"

void ui_init();
void ui_cleanup();
char* ui_get_search_query();
void ui_show_loading();
void ui_show_error(const char *message);
AnimeInfo* ui_select_anime(SearchResult *results);
Episode* ui_select_episode(AnimeInfo *anime);
void ui_play_episode(StreamInfo *stream);

#endif /* UI_H */