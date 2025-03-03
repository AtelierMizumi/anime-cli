// manga_ui.h - Header file for the manga UI functionality

#ifndef MANGA_UI_H
#define MANGA_UI_H

#include <stdbool.h>
#include "../api/manga.h"

/**
 * Prompts the user for a search query
 * 
 * @return A dynamically allocated string containing the search query (must be freed)
 */
char* manga_ui_get_search_query();

/**
 * Allows the user to select a manga from search results
 * 
 * @param results The search results from which to select
 * @return A pointer to MangaInfo for the selected manga, or NULL if cancelled
 */
void* manga_ui_select_manga(SearchResult *results);

/**
 * Allows the user to select a chapter from a manga
 * 
 * @param manga The manga information containing chapters
 * @return A pointer to ChapterPages for the selected chapter, or NULL if cancelled
 */
void* manga_ui_select_chapter(MangaInfo *manga);

/**
 * View the pages of a manga chapter using an external image viewer
 * 
 * @param pages The chapter pages to view
 */
void manga_ui_view_chapter(ChapterPages *pages);

/**
 * Main interaction loop for the manga UI
 */
void manga_ui_main_loop();

#endif /* MANGA_UI_H */